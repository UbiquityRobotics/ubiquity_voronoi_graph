#################################################################
## This message is used to transport a graph between two nodes.
## Each edge has a set of points and a width describing the
## shape of it and additionally successors and predecessors
## to describe the overall structure of the graph 
#################################################################

Header header               # Header for Timestamp and Id
geometry_msgs/Pose origin   # The origin of the map (points in vertices do not know how map is moved)
Vertex[] vertices           # The edges in the graph
