#################################################################
## A single edge in a graph 
## Each edge of the same graph must have a unique id. 
## Successors share current edge end point
## Predeseccors share current edge start point
#################################################################
uint32 id                   # Vertex id
bool valid                  # true if it can be used for planning
geometry_msgs/Point[] path  # points describing a path from the edge start point to the edge end point
                            # the first point in the array reprecents the start point and the last the end point
                            # this points can also be used by the vehciles local path following algorithm
uint32 weight               # the weight of the vertex (currently number of point describing the edge edge)
float32 width               # free space next to the vertex
uint32[] successors         # edges to successors
uint32[] predecessors       # edges to predecessor
 
