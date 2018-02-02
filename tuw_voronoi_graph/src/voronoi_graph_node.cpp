#include <ros/ros.h>
#include <nav_msgs/OccupancyGrid.h>
#include <tuw_voronoi_map/voronoi_path_generator.h>
#include <tuw_voronoi_graph/voronoi_graph_node.h>
#include <voronoi_segmentation/voronoi_graph_generator.h>
#include <memory>
#include <boost/functional/hash.hpp>
#include <tuw_multi_robot_msgs/VoronoiGraph.h>
#include <string>

void publishTf(std::string mapName);

bool allowPublish = false;

int main(int argc, char** argv)
{

    ros::init(argc, argv, "voronoi_graph_node");     /// initializes the ros node with default name
    ros::NodeHandle n;

    voronoi_graph::VoronoiGeneratorNode mapNode(n);

    ros::Rate r(0.5);

    ROS_INFO("Initialization done!");

    while(ros::ok())
    {
        ros::spinOnce();

        if(allowPublish)
        {
            mapNode.publishMap();
            mapNode.publishSegments();
        }

        r.sleep();
    }

    return 0;
}


namespace voronoi_graph
{
    VoronoiGeneratorNode::VoronoiGeneratorNode(ros::NodeHandle & n) :  voronoi_graph::VoronoiPathGenerator(), VoronoiGraphGenerator(), n_(n), n_param_("~"), Serializer()
    {

        topicGlobalMap_ = "/map";
        n.param("map_topic", topicGlobalMap_, topicGlobalMap_);

        smoothing_ = 15;
        n.param("map_smoothing", smoothing_, smoothing_);
        
        
        topicVoronoiMap_ = "voronoi_map";
        n.param("voronoi_topic", topicVoronoiMap_, topicVoronoiMap_);

        topicSegments_ = "/segments";
        n.param("segments_topic", topicSegments_, topicSegments_);

        frameGlobalMap_ = "map";
        n.param("map_frame", frameGlobalMap_, frameGlobalMap_);
        frameVoronoiMap_ = "voronoi_map";
        n.param("voronoi_frame", frameVoronoiMap_, frameVoronoiMap_);

        path_length_ = 0.9;   //meter
        n_param_.param("segment_length", path_length_, path_length_);

        crossingOptimization_ = 0.2;
        n.param("opt_crossings", crossingOptimization_, crossingOptimization_);
        
        endSegmentOptimization_ = 0.4;
        n.param("opt_end_segments", endSegmentOptimization_, endSegmentOptimization_);
        endSegmentOptimization_ = std::min<float>(endSegmentOptimization_, 0.7 * path_length_);
        
        graphPath_ = "./";
        n.param("graph_path", graphPath_, graphPath_);       
        if(graphPath_.back() != '/')
            graphPath_ += "/";
        
        
        subMap_       = n.subscribe(topicGlobalMap_, 1, &VoronoiGeneratorNode::globalMapCallback, this);
        //pubVoronoiMap_    = n.advertise<nav_msgs::OccupancyGrid>(topicVoronoiMap_, 1);                        //DEBUG
        pubSegments_  = n.advertise<tuw_multi_robot_msgs::VoronoiGraph>(topicSegments_, 1);

        
        //ros::Rate(1).sleep();
        ros::spinOnce();
    }

    void VoronoiGeneratorNode::globalMapCallback(const nav_msgs::OccupancyGrid::ConstPtr& _map)
    {
        std::vector<signed char> map = _map->data;

        Eigen::Vector2d origin;
        origin[0] = _map->info.origin.position.x;
        origin[1] = _map->info.origin.position.y;

        size_t new_hash = getHash(map, origin, _map->info.resolution);

        if(new_hash != current_map_hash_)
        {          
            if(!loadGraph(new_hash))
            {
                ROS_INFO("Graph generator: Graph not found! Generating new one!");
                createGraph(_map, new_hash);
            }
            else
            {
                ROS_INFO("Graph generator: Graph loaded from memory");
            }
            current_map_hash_ = new_hash;
            
            allowPublish = true;
        }
    }

    void VoronoiGeneratorNode::createGraph(const nav_msgs::OccupancyGrid::ConstPtr& _map, size_t _map_hash)
    {
            std::vector<signed char> map = _map->data;
            
            ROS_INFO("Graph generator: Computing distance field ...");
            origin_[0] = _map->info.origin.position.x;
            origin_[1] = _map->info.origin.position.y;
            resolution_ = _map->info.resolution;

            cv::Mat m(_map->info.height, _map->info.width, CV_8SC1, map.data());
            prepareMap(m, map_, smoothing_);           
            computeDistanceField(map_, distField_);
            
            ROS_INFO("Graph generator: Computing voronoi graph ...");
            computeVoronoiMap(distField_, voronoiMap_);

            ROS_INFO("Graph generator: Generating graph ...");
            potential.reset(new float[m.cols * m.rows]);
            float pixel_path_length = path_length_ / resolution_;
            segments_ = calcSegments(m, distField_, voronoiMap_, potential.get(), pixel_path_length, crossingOptimization_ / resolution_, endSegmentOptimization_ / resolution_);

            //Check Directroy
            save(graphPath_ + std::to_string(_map_hash) + "/", segments_, origin_, resolution_);    
            ROS_INFO("Graph generator: Created new Graph %lu", _map_hash);
            
    }

    bool VoronoiGeneratorNode::loadGraph(std::size_t hash)
    {
            return load(graphPath_ + std::to_string(hash) + "/", segments_, origin_, resolution_);
    }

    
    void VoronoiGeneratorNode::publishSegments()
    {
        tuw_multi_robot_msgs::VoronoiGraph graph;
        graph.header.frame_id = "map";
        graph.header.seq = 0;
        graph.header.stamp = ros::Time::now();

        graph.resolution = resolution_;

        graph.origin.position.x = -origin_[0];
        graph.origin.position.y = -origin_[1];


        for(auto it = segments_.begin(); it != segments_.end(); ++it)
        {
            tuw_multi_robot_msgs::Vertex seg;
            seg.header.frame_id = "map";
            seg.header.seq = 0;
            seg.header.stamp = ros::Time::now();

            seg.id = (*it)->GetId();
            seg.length = (*it)->GetLength();
            seg.minPathSpace = (*it)->GetMinPathSpace();

            std::vector< Eigen::Vector2d >  path = (*it)->GetPath();

            for(int i = 0; i < path.size(); i++)
            {
                geometry_msgs::Point pos;
                pos.x = path[i][0];
                pos.y = path[i][1];
                pos.z = 0;

                seg.path.push_back(pos);
            }

            //ROS_INFO("distORIG: %i/%i", (*it)->GetPredecessors().size(), (*it)->GetSuccessors().size());
            std::vector< std::shared_ptr< Segment > > predecessors = (*it)->GetPredecessors();

            for(int i = 0; i < predecessors.size(); i++)
            {
                seg.predecessor.push_back(predecessors[i]->GetId());
            }

            std::vector< std::shared_ptr< Segment > > successors = (*it)->GetSuccessors();

            for(int i = 0; i < successors.size(); i++)
            {
                seg.successors.push_back(successors[i]->GetId());
            }

            graph.segments.push_back(seg);
        }
        pubSegments_.publish(graph);
    }


    void VoronoiGeneratorNode::publishMap() //DEBUG
    {
        nav_msgs::OccupancyGrid grid;
        // Publish Whole Grid
        grid.header.frame_id = "map";
        grid.header.stamp = ros::Time::now();
        grid.info.resolution = resolution_;

        int nx = map_.cols;
        int ny = map_.rows;

        grid.info.width = nx;
        grid.info.height = ny;

        double wx, wy;
        //costmap_->mapToWorld(0, 0, wx, wy);
        grid.info.origin.position.x = origin_[0];
        grid.info.origin.position.y =  origin_[1];
        grid.info.origin.position.z = 0.0;
        grid.info.origin.orientation.w = 1.0;

        grid.data.resize(nx * ny);


        for(unsigned int i = 0; i < grid.data.size(); i++)
        {
            grid.data[i] = (map_.data[i]);

        }

        //pubVoronoiMap_.publish(grid);
    }

    std::size_t VoronoiGeneratorNode::getHash(const std::vector<signed char> &_map, Eigen::Vector2d _origin, float _resolution)
    {
        std::size_t seed = 0;

        boost::hash_combine(seed, _origin[0]);
        boost::hash_combine(seed, _origin[1]);
        boost::hash_combine(seed, _resolution);

        for(const signed char & val : _map)
        {
            boost::hash_combine(seed, val);
        }

        return seed;
    }
}
