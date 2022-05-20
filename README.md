# ubiquity\_voronoi\_graph

<em>This package is a slightly modified version of [tuw_voronoi_graph](https://github.com/tuw-robotics/tuw_multi_robot/tree/master/tuw_voronoi_graph). It also utilizes parts of [tuw_multi_robot_rviz](https://github.com/tuw-robotics/tuw_multi_robot/tree/master/tuw_multi_robot_rviz) and [tuw_msgs](https://github.com/tuw-robotics/tuw_msgs)</em>  

<br>

This package generates a voronoi graph out of a map.  
Provides the messages for transporting the graph between nodes and the rviz plugin needed for visualization.

<br>

## voronoi\_graph\_node
Receives a pixel map ([occupancy\_grid](http://docs.ros.org/api/nav_msgs/html/msg/OccupancyGrid.html)) and converts it into a voronoi graph describing the original map.

For each graph a hash number is generated based on the map name.
Each graph is saved in 3 separated files in cfg/graph/name_of_map/generated_hash_number: [graphData.xml](#graphdataxml-contains-the-voronoi-graph-edges-data), [graphInfo.xml](#graphinfoxml-contains-the-voronoi-graph-meta-data) and [treeInfo.xml](#treeinfoxml). These files can be used to load the graph instead of generating one. Right now this function is not available - see [TODO](#todo)
  
<br>

### Subscribed Topics

`~map` (`nav_msgs::occupancy_grid`)
    The map used for planning (used for matching odom pose to graph)

### Published Topics

`~segments` (`tuw_multi_robot_msgs::VoronoiGraph`)
    The generated graph (edges published on this topic do not take into account the origin of the map)

### Parameters

`~map_topic` (`string` default: "/map")

     Sets the topic where the map is read from

`~map_smoothing` (`int` default: "15")

     Sets the parameter to smooth the map making cleaner voronoi graphs. (Opencv sometimes segfaults during this process (change this param if so). Keeping this param at zero never produces any segfault.

`~segments_topic` (`string` default: "/segments")

     The topic where the graph is published.

`~segment_length` (`float` default: "0.9")

     Defines the length of a graph segment.

`~crossing_opimization` (`float` default: "0.2")

     Crossings which have less distance than this value are merged together

`~end_segment_optimization` (`float` default: "0.4")

     End segments (has only one neighbor) which are shorter than this value are removed.

`~graph_path` (`string` default: ".") 

     Defines the path were the generated graphs are saved

`~custom_graph_path` (`string` default: "") 

     If this param is set a custom generated graph is loded from this path

<br>

## graphData.xml (contains the voronoi graph edges data)  
```xml
    <id>2</id>
    <predecessorLength>2</predecessorLength>
    <successorLength>1</successorLength>
    <minDistance>1.719003296e+01</minDistance>
    <pointLength>65</pointLength>
    <item>0</item>
    <item>1</item>
    <item>3</item>
    <item>
        <x>4.210000000e+02</x>
        <y>4.220000000e+02</y>
    </item>
    <item>
        <x>4.200000000e+02</x>
        <y>4.220000000e+02</y>
    </item>
    <item>
        <x>4.190000000e+02</x>
        <y>4.210000000e+02</y>
    </item>
```
- `<id>` = edge id  
- `<predecessorLength>` = num of predecessors  
- `<successorLength>` = num of successors  
- `<minDistance>` = 
- `<pointLength>` = num of points that describe the edge  
- `<item>num</item>` = ids of predecessors and successors using the num of predecessors as a split point  
- `<item><x><y></item>`the coordinates of points describing the edge  
  
## graphInfo.xml (contains the voronoi graph meta data)
```xml
<GraphInfo class_id="0" tracking_level="0" version="0">
	<Origin class_id="1" tracking_level="0" version="0">
		<x>-1.500000000e+01</x>
		<y>-1.500000000e+01</y>
	</Origin>
	<Resolution>5.000000075e-02</Resolution>
	<NumberOfSegments>20</NumberOfSegments>
</GraphInfo>
```
- `<Origin>` = map origin coordinates
- `<Resolution>` = map resolution
- `<NumberOfSegments>` = number of segments(edges) in the graph  
  

## treeInfo.xml
```xml
<TreeInfo class_id="0" tracking_level="0" version="0">
	<Length>20</Length>
	<item>2</item>
	<item>2</item>
	<item>2</item>
	<item>1</item>
	<item>2</item>
	<item>1</item>
	<item>2</item>
	<item>2</item>
	<item>2</item>
	<item>1</item>
```
- `<Length>` = number of segments(edges) in the graph  
-  the `<Length>` tag is follow by `3*number of segments(edges)` of `<item>` tags, signifying *predecessorLength*, *successorLength* and *pointLength* of each edge

<br>

## ubiquity\_voronoi\_rviz
Presents rviz plugins for visualizing generated graphs. 

<br>

## TODO
- error when trying to read already generated map from file
- parameters not impacting the graph as they should

