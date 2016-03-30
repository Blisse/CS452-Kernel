#pragma once

typedef enum {
  NODE_NONE,
  NODE_SENSOR,
  NODE_BRANCH,
  NODE_MERGE,
  NODE_ENTER,
  NODE_EXIT,
} node_type;

#define DIR_AHEAD 0
#define DIR_STRAIGHT 0
#define DIR_CURVED 1

struct track_node;
typedef struct track_node TRACK_NODE;
typedef struct track_edge TRACK_EDGE;

struct track_edge {
  TRACK_EDGE *reverse;
  TRACK_NODE *src, *dest;
  int dist;             /* in millimetres */
};

struct track_node {
  const char* name;
  node_type type;
  int num;              /* sensor or switch number */
  TRACK_NODE* reverse;  /* same location, but opposite direction */
  TRACK_EDGE edge[2];

  // ONLY USED FOR PATH FINDING
  int path_distance;
  TRACK_NODE* path_parent;
};
