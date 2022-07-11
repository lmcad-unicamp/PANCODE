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
#ifndef TARGET
#define TARGET

/* Graph vertices are represented by objects of type vertex. */
#define vertex int

/* The adjacency list of a vertex v is composed by nodes of type node. Each node of the list corresponds to an arc and contains a neighbour w of v and the address of the next targetNode in the list. A targetLink is a pointer to a node. */
typedef struct targetNode *targetLink;

class TargetGraph {

public:
	// constructor
	TargetGraph(int n, int nThreads);

	// copy constructor
	TargetGraph(const TargetGraph &graphToCopy);

	/* ADJACENCY LIST REPRESENTATION: the function GRAPHinsertArc() inserts an arc v-w in graph G. The function supposes that v and w are distinct, positive, and smaller than G->V. If the graph already has an arc v-w, the function does not do anything. */
	void insertArc(vertex v, vertex w);

	/* Updates vertex weights */
	//void targetGraphUpdateComputationalWeight(targetGraph G, vertex v, int computational_weight);

	/* Updates memory weights */
	void setMemoryWeight(vertex v, int memoryWeight);

	/* Updates computational weights */
	void setComputationalWeight(vertex v, int compWeight);

	/* Updates connection weights */
	void setConnectionWeight(vertex src, vertex dst, int connWeight);

	/* Updates assigned */
	void setAssigned(int partition, int device);
	void setAssignedPerThread(int partition, int device, int thread);

	int getMemory(int machine) const;

	int getMinMemory() const;

	int getComputationalWeight(int machine) const;

	int getAssigned(int machine) const;
	
	int getConnectionWeight(int src, int dst) const;

	int getNumberOfVertices() const;

	void printGraph() const;
	void printGraphHeader() const;

	~TargetGraph();

private:
   int V; 
   int A;
   int numberOfThreads;
   int *computationalWeight;
   int *memory;
   int *assigned;
   int **assignedPerThread;
   int **connectionMatrix;
   targetLink *adj; 
};

#endif // TARGET

#ifndef TARGET_NODE
#define TARGET_NODE
struct targetNode { 
   vertex w; 
   targetLink next; 
};
#endif

