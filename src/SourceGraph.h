/***************************************************************************
 *   Copyright (C) 2020 by Fabíola Martins Campos de Oliveira 		   	   *
 *   fabiola.bass@gmail.com			                           			   *
 *                       						   						   *
 *   This file is part of KLP, DN²PCIoT, and MDN²PCIoT.  				   *
 *                                      		   		   				   *
 *   KLP, DN²PCIoT, and MDN²PCIoT is free software: you can redistribute   *
 *   it and/or modify it under the terms of the GNU General Public License *
 *   as published by the Free Software Foundation, either version 3 of the * 
 *   License, or (at your option) any later version.				       *
 *									   									   *
 *   KLP, DN²PCIoT, and MDN²PCIoT is distributed in the hope that it will  *
 *   be useful,	but WITHOUT ANY WARRANTY; without even the implied 		   *	
 *   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See * 
 *   the GNU General Public License for more details.			   		   *
 *									   									   *
 *   You should have received a copy of the GNU General Public License     *
 *   long with KLP, DN²PCIoT, and MDN²PCIoT.  If not, see 				   *
 *   <http://www.gnu.org/licenses/>.     								   *
 ***************************************************************************/

// https://www.ime.usp.br/~pf/algoritmos_para_grafos/aulas/graphdatastructs.html
#ifndef GRAPH
#define GRAPH

/* Graph vertices are represented by objects of type vertex. */
#define vertex int

/* The adjacency list of a vertex v is composed by nodes of type node. Each node of the list corresponds to an arc and contains a neighbour w of v and the address of the next node in the list. A link is a pointer to a node. */
typedef struct node *links;

typedef struct node_matrix *nodeMatrix;

class SourceGraph {

public:
	/* ADJACENCY LIST REPRESENTATION: the function GRAPHinit() builds a graph with vertices 0 1 .. V-1 and no arc. */
	SourceGraph(int n, int nLayers, int nPartitions = 2, bool type = false);

	// copy constructor
	SourceGraph(const SourceGraph &graphToCopy);

	/* The function NEWnode() receives a vertex w and the address next of a node and returns the address of a new node such that a->w == w and a->next == next. */
	//static link NEWnode(vertex w, link next, int edgeW);
	links NEWnode(vertex w, links next, int edgeW, /*int redN,*/ int src=-1, int srcMvl=-1);
	/* ADJACENCY LIST REPRESENTATION: the function GRAPHinsertArc() inserts an arc v-w in graph G. The function supposes that v and w are distinct, positive, and smaller than G->V. If the graph already has an arc v-w, the function does not do anything. */
	void insertArc(vertex v, vertex w, int edgeW, /*int redN,*/ int src=1);
	void insertArcSum(vertex v, vertex w, int edgeW, int src);
	bool insertArcSrc(vertex v, vertex w, int edgeW, /*int redN,*/ int src);
	bool insertArcSrcMlvl(vertex v, vertex w, int edgeW, /*int redN,*/ int src);
	bool insertArcSrcMlvlPartitionGraph(vertex v, vertex w, int edgeW, /*int redN,*/ int src, int srcMlvl);
	/* Updates vertex weights */
	void setVertexWeight(vertex v, int vertexW);
	int getVertexWeight(int v) const {
		return vertexWeight[v];
	}
	/* Updates memory weights */
	void setMemoryWeight(vertex v, int memoryWeight);
	/* Updates flags */
	int getNumberOfVertices() const {
		return V;
	}
	int getNumberOfEdges() const {
		return A;
	}
	links getAdjOfVertex(int i) const {
		return adj[i];
	}

	void setFlags(int vertexLabel, int edgeWeight, int vertexWeight, int memory);
	int getEnableVertexLabels() const {
		return enableVertexLabels;
	}
	int getEnableEdgeWeights() const {
		return enableEdgeWeights;
	}
	int getEnableMemory() const {
		return enableMemory;
	}
	int getEnableVertexWeights() const {
		return enableVertexWeights;
	}

	void printGraph() const;
	void printGraphSrc() const;
	void printGraphMlvl() const;
	void printGraphHeader() const;

	int getMemory(vertex v) const {
		return memory[v];
	}
	bool removeArcSrc(vertex v, vertex w, int src);
	//void setAdjMatrix(vertex v, vertex w, double edgeW);

	void setNumberOfLayers(int num);
	int getNumberOfLayers() const {
		return numberOfLayers;
	}
	void setLayerInitialPos(int i, int pos);
	int getLayerInitialPos(int i) const {
		return layerInitialPos[i];
	}
	void setLayerWidth(int i, int pos);
	int getLayerWidth(int i) const {
		return layerWidth[i];
	}
	void setLayerHeight(int i, int pos);
	int getLayerHeight(int i) const {
		return layerHeight[i];
	}
	void setOriginalDepth(int i, int pos);
	int getOriginalDepth(int i) const {
		return originalDepth[i];
	}
	/*void setRedundantNeurons(int i, int pos);
	int getRedundantNeurons(int i) const {
		return [i]->redundantNeurons;
	}*/
	void setSharedParam(int i, int numP);
	int getSharedParam(int layer) const {
		return sharedParam[layer];
	}

	int getSharedParams(int u) const;

	int getMatch(int i) const {
		return match[i];
	}
	void setMatch(int u, int v);

	void setEdgeMatch(links node);

	void setMap(int u, int v, int label);
	int getMap(int u) const {
		return map[u];
	}

	void setVertexDegree(int v, int deg);
	int getVertexDegree(int v) const {
		return degree[v];
	}

	int getLayerOfVertex(int v) const;

	void setNodeSharedParam(int v, int layer);

	int getNodeSharedParam(int v, int layer) {
		return sharedParamPerVertex[v][layer];
	}

	void setNodeSharedParamArray(int v, bool *shrdppv);

	void getSharedParamArray(int v, bool *shrdppv) const;

	void getSharedParamPerLayer(int *shrdppl) const;

	void setSharedParamPerLayer(int *shrdppl) const;

	//int getAdjMatrixEdgeWeight(int i, int j);

	~SourceGraph();

private:
	int V;
	int A; 
	// type = false (SourceGraph), true (partitionGraph)
	bool type;

	int enableVertexLabels;
	int enableEdgeWeights;
	int enableVertexWeights;
	int enableMemory;

	int numberOfLayers;
	int *layerInitialPos;
	int *sharedParam;
	int *layerWidth;
	int *layerHeight;
	int *originalDepth;

	int *vertexWeight;
	int *memory;
	int *match;
	int *map;
	int *degree;
	bool **sharedParamPerVertex;
	int *adjMatrixSize;
	node_matrix **adjMatrix;
	links *adj;
};

#endif

#ifndef NODE
#define NODE
struct node { 
	vertex w; 
	int edgeWeight;
	int source;
	int sourceMlvl;
	//int redundantNeurons;
	int matched;
	links next; 
};
#endif

#ifndef NODE_MATRIX
#define NODE_MATRIX
struct node_matrix{
	int edgeWeight;
	int source;
	int sourceMlvl;
	int matched;
};
#endif

