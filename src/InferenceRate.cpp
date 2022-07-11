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

#include <cstddef>
#include <iostream>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include "ReadInputFiles.h"
#include "GenericPartitioningAlgorithm.h"
#include "InferenceRate.h"

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;

InferenceRate::InferenceRate(SourceGraph *srcG, int nPartitions, bool movingNodes, bool exchangingNodes, TargetGraph *tgtG, int nVertices, int forcedInput, bool initPart, char *verbose, char *initPartFile, int numberOfThreads, bool multilevel, int *multilevelPart, int higherLevelNumberOfVertices, bool objFunc)   
	: GenericPartitioningAlgorithm(nVertices, nPartitions, movingNodes, exchangingNodes, numberOfThreads, objFunc),
	target(tgtG), sourceG(srcG) 
{
	validArray = (int *) calloc(nPartitions, sizeof(int));
	if(validArray == NULL) {
		cout << "\n validArray could not be allocated! \n";
		exit(EXIT_FAILURE);	
	}

	validArrayPerThread = (int **) malloc(getNumberOfThreads() * sizeof(int *));
	if (validArrayPerThread == NULL) {
		cout << "\n validArrayPerThread could not be allocated! \n";
		exit(EXIT_FAILURE);	
	}
	for (int i = 0; i < getNumberOfThreads(); i++) {
		validArrayPerThread[i] = (int *) malloc(nPartitions * sizeof(int));
		if (validArrayPerThread[i] == NULL) {
			cout << "\n validArrayPerThread[i] could not be allocated! \n";
			exit(EXIT_FAILURE);	
		}		
	}
	for (int i = 0; i < getNumberOfThreads(); i++) {
		for (int j = 0; j < nPartitions; j++) {
			validArrayPerThread[i][j] = 0;
		}
	}

	validIndexes = (int *) calloc(nPartitions, sizeof(int));
	if(validIndexes == NULL) {
		cout << "\n validIndexes could not be allocated! \n";
		exit(EXIT_FAILURE);	
	}

	validIndexesPerThread = (int **) malloc(getNumberOfThreads() * sizeof(int *));
	if (validIndexesPerThread == NULL) {
		cout << "\n validIndexesPerThread could not be allocated! \n";
		exit(EXIT_FAILURE);	
	}
	for (int i = 0; i < getNumberOfThreads(); i++) {
		validIndexesPerThread[i] = (int *) malloc(nPartitions * sizeof(int));
		if (validIndexesPerThread[i] == NULL) {
			cout << "\n validIndexesPerThread[i] could not be allocated! \n";
			exit(EXIT_FAILURE);	
		}		
	}

	targetIndexes = (int *) calloc(nPartitions, sizeof(int));
	if(targetIndexes == NULL) {
		cout << "\n targetIndexes could not be allocated! \n";
		exit(EXIT_FAILURE);	
	}
	for (int i = 0; i < nPartitions; i++) {
		validIndexes[i] = i;
		targetIndexes[i] = i;
	}

	/*sourceMem = (int *) malloc(getNumberOfPartitions() * sizeof(int));
	if(sourceMem == NULL) {
		cout << "\n sourceMem could not be allocated! \n";
		exit(EXIT_FAILURE);	
	}*/

	sharedMemoryPerPartition = (bool **) malloc(getNumberOfPartitions() * sizeof(bool *));
	if (sharedMemoryPerPartition == NULL) {
		cout << "\n sharedMemoryPerPartition could not be allocated! \n";
		exit(EXIT_FAILURE);	
	}
	for (int i = 0; i < getNumberOfPartitions(); i++) {
		sharedMemoryPerPartition[i] = (bool *) malloc(sourceG->getNumberOfLayers() * sizeof(bool));
		if (sharedMemoryPerPartition[i] == NULL) {
			cout << "\n sharedMemoryPerPartition[i] could not be allocated! \n";
			exit(EXIT_FAILURE);	
		}		
	}
	for (int i = 0; i < getNumberOfPartitions(); i++) {
		for (int j = 0; j < sourceG->getNumberOfLayers(); j++) {
			sharedMemoryPerPartition[i][j] = false;
		}
	}

	sharedMemoryPerPartitionPerThread = (bool ***) malloc(getNumberOfThreads() * sizeof(bool **));
	if (sharedMemoryPerPartitionPerThread == NULL) {
		cout << "\n sharedMemoryPerPartitionPerThread could not be allocated! \n";
		exit(EXIT_FAILURE);	
	}
	for (int i = 0; i < getNumberOfThreads(); i++) {
		sharedMemoryPerPartitionPerThread[i] = (bool **) malloc(nPartitions * sizeof(bool *));
		if (sharedMemoryPerPartitionPerThread[i] == NULL) {
			cout << "\n sharedMemoryPerPartitionPerThread[i] could not be allocated! \n";
			exit(EXIT_FAILURE);	
		}		
	}
	for (int i = 0; i < getNumberOfThreads(); i++) {
		for (int j = 0; j < nPartitions; j++) {
			sharedMemoryPerPartitionPerThread[i][j] = (bool *) malloc(sourceG->getNumberOfLayers() * sizeof(bool));
			if (sharedMemoryPerPartitionPerThread[i][j] == NULL) {
						cout << "\n sharedMemoryPerPartitionPerThread[i][j] could not be allocated! \n";
						exit(EXIT_FAILURE);	
			}		
		}
	}
	for (int t = 0; t < getNumberOfThreads(); t++) {
		for (int i = 0; i < getNumberOfPartitions(); i++) {
			for (int j = 0; j < sourceG->getNumberOfLayers(); j++) {
				sharedMemoryPerPartitionPerThread[t][i][j] = false;
			}
		}
	}

	setInitialPartitionings(forcedInput, initPart, initPartFile, multilevel, multilevelPart, higherLevelNumberOfVertices);

	// to use in validPartitioning()
	sortedTargetMem = (int *) malloc(getNumberOfPartitions() * sizeof(int));
	if(sortedTargetMem == NULL) {
		cout << "\n sortedTargetMem could not be allocated! \n";
		exit(EXIT_FAILURE);	
	}
	for (int i = 0; i < getNumberOfPartitions(); i++) {
		sortedTargetMem[i] = target->getMemory(i);
	}
	mergeSortWithIndexes(sortedTargetMem, 1, getNumberOfPartitions(), targetIndexes);
	// this makes the sorting not stable; to have a stable sort comment this block and use Source_graph->V/2 - 1 - a (or b) in FindMaxReductionInCost()
	for (int a = 0; a < getNumberOfPartitions()/2; a++) {
		int b = getNumberOfPartitions() - a - 1;
		int sortaux;

		sortaux = sortedTargetMem[a];
		sortedTargetMem[a] = sortedTargetMem[b];
		sortedTargetMem[b] = sortaux;

		sortaux = targetIndexes[a];
		targetIndexes[a] = targetIndexes[b];
		targetIndexes[b] = sortaux;
	}

	// call this function in order to assign the correct values to "assigned"
	bool valid = validPartitioning(getInitialPartitioning());

	partitionGraphPerThread = (SourceGraph **) malloc(getNumberOfThreads() * sizeof(SourceGraph *));
	if (partitionGraphPerThread == NULL) {
		cout << "\n partitionGraphPerThread could not be allocated! \n";
		exit(EXIT_FAILURE);	
	}
	for (int i = 0; i < getNumberOfThreads(); i++)
		partitionGraphPerThread[i] = new SourceGraph(getNumberOfPartitions(), getNumberOfPartitions());

	// build partitionGraph based on SourceG (input graph); eliminates redundant edges	
	partitionGraph = new SourceGraph(getNumberOfPartitions(), getNumberOfPartitions());

	currentCost = 0;
	bool inserted;
	int cost, costRed, costMETIS = 0;
	for (int i = 0; i < getNumberOfVertices(); i++) {
		for (links nodeAdj = sourceG->getAdjOfVertex(i); nodeAdj != NULL; nodeAdj = nodeAdj->next) {
			if (getCurrentPartitioning(i) != getCurrentPartitioning(nodeAdj->w) /*&& nodeAdj->w > i*/) {
				inserted = partitionGraph->insertArcSrcMlvlPartitionGraph(getCurrentPartitioning(i), getCurrentPartitioning(nodeAdj->w), nodeAdj->edgeWeight, /*nodeAdj->redundantNeurons,*/ i, nodeAdj->sourceMlvl);
				costMETIS += nodeAdj->edgeWeight;
				if (inserted) {
					currentCost += nodeAdj->edgeWeight;
					//currentCost += cost;
					if ((strcmp(verbose, "verb")) == 0) {
						//cout << "\n i: " << i << " nodeAdj: " << nodeAdj->w << " eW: " << nodeAdj->edgeWeight << " redN: " << nodeAdj->redundantNeurons << " cost: " << currentCost;
					}
				}
			}
		}
	}
	if ((strcmp(verbose, "verb")) == 0) {
		//partitionGraph->printGraphSrc();
	}

	// stores the computational power needed by each partition
	deviceInferenceRate = (double *) malloc(getNumberOfPartitions() * sizeof(double));
	if(deviceInferenceRate == NULL) {
		cout << "\n deviceInferenceRate could not be allocated! \n";
		exit(EXIT_FAILURE);	
	}

	deviceInferenceRatePerThread = (double **) malloc(getNumberOfThreads() * sizeof(double *));
	if(deviceInferenceRatePerThread == NULL) {
		cout << "\n deviceInferenceRatePerThread could not be allocated! \n";
		exit(EXIT_FAILURE);	
	}
	for (int v = 0; v < getNumberOfThreads(); v++) {
		deviceInferenceRatePerThread[v] = (double *) malloc(getNumberOfPartitions() * sizeof(double));
		if (deviceInferenceRatePerThread[v] == NULL) {
			std::cout << "deviceInferenceRatePerThread[v] could not be allocated! \n";
			exit(EXIT_FAILURE);
		}
	}
	for (int i = 0; i < getNumberOfThreads(); i++) {
		for (int j = 0; j < getNumberOfPartitions(); j++)
			deviceInferenceRatePerThread[i][j] = 0;		
	}

	partitionComputationalWeight = (double *) malloc(getNumberOfPartitions() * sizeof(double));
	if(partitionComputationalWeight == NULL) {
		cout << "\n partitionComputationalWeight could not be allocated! \n";
		exit(EXIT_FAILURE);	
	}
	for (int i = 0; i < getNumberOfPartitions(); i++) {
		partitionComputationalWeight[i] = 0;		
	}

	partitionComputationalWeightPerThread = (double **) malloc(getNumberOfThreads() * sizeof(double *));
	if(partitionComputationalWeightPerThread == NULL) {
		cout << "\n partitionComputationalWeightPerThread could not be allocated! \n";
		exit(EXIT_FAILURE);	
	}
	for (int v = 0; v < getNumberOfThreads(); v++) {
		partitionComputationalWeightPerThread[v] = (double *) malloc(getNumberOfPartitions() * sizeof(double));
		if (partitionComputationalWeightPerThread[v] == NULL) {
			std::cout << "deviceInferenceRatePerThread[v] could not be allocated! \n";
			exit(EXIT_FAILURE);
		}
	}
	for (int i = 0; i < getNumberOfThreads(); i++) {
		for (int j = 0; j < getNumberOfPartitions(); j++)
			partitionComputationalWeightPerThread[i][j] = 0;		
	}

	// stores the inference rate of each connection between each device
	connectionMatrixInferenceRate = (double **) malloc(getNumberOfPartitions() * sizeof(double *));
	if(connectionMatrixInferenceRate == NULL) {
		cout << "\n connectionMatrixInferenceRate could not be allocated! \n";
		exit(EXIT_FAILURE);	
	}
	for (int v = 0; v < getNumberOfPartitions(); v++) {
		connectionMatrixInferenceRate[v] = (double *) malloc(getNumberOfPartitions() * sizeof(double));
		if (connectionMatrixInferenceRate[v] == NULL) {
			std::cout << "connectionMatrixInferenceRate[v] could not be allocated! \n";
			exit(EXIT_FAILURE);
		}
	}
	for (int v = 0; v < getNumberOfPartitions(); ++v) {
		for (int i = 0; i < getNumberOfPartitions(); i++) {
			connectionMatrixInferenceRate[v][i] = 0;
		}
	}

	connectionMatrixInferenceRatePerThread = (double ***) malloc(getNumberOfThreads() * sizeof(double **));
	if(connectionMatrixInferenceRatePerThread == NULL) {
		cout << "\n connectionMatrixInferenceRatePerThread could not be allocated! \n";
		exit(EXIT_FAILURE);	
	}
	for (int v = 0; v < getNumberOfThreads(); v++) {
		connectionMatrixInferenceRatePerThread[v] = (double **) malloc(getNumberOfPartitions() * sizeof(double *));
		if (connectionMatrixInferenceRatePerThread[v] == NULL) {
			std::cout << "connectionMatrixInferenceRatePerThread[v] could not be allocated! \n";
			exit(EXIT_FAILURE);
		}
	}
	for (int v = 0; v < getNumberOfThreads(); v++) {
		for (int i = 0; i < getNumberOfPartitions(); i++) {
			connectionMatrixInferenceRatePerThread[v][i] = (double *) malloc(getNumberOfPartitions() * sizeof(double));
			if (connectionMatrixInferenceRatePerThread[v][i] == NULL) {
				std::cout << "connectionMatrixInferenceRatePerThread[v][i] could not be allocated! \n";
				exit(EXIT_FAILURE);
			}
		}
	}
	for (int t = 0; t < getNumberOfThreads(); t++)
		for (int v = 0; v < getNumberOfPartitions(); ++v) {
			for (int i = 0; i < getNumberOfPartitions(); i++) {
				connectionMatrixInferenceRatePerThread[t][v][i] = 0;
			}
		}

	cout << "\n costMETIS: " << costMETIS << ", cost: " << currentCost;
	//cout << "\n cost: " << currentCost;
	currentCost = computeCost(getInitialPartitioning(), false, true);
	//cout << "\n cost: " << currentCost;
}

void InferenceRate::setSourceGraph(SourceGraph *srcG) {
	sourceG = srcG;
}

// Seed the random number generator.
void InferenceRate::seed_rng(void)
{
    int fp = open("/dev/random", O_RDONLY);
    if (fp == -1) abort();
    unsigned seed;
    unsigned pos = 0;
    while (pos < sizeof(seed)) {
        int amt = read(fp, (char *) &seed + pos, sizeof(seed) - pos);
        if (amt <= 0) abort();
        pos += amt;
    }
    srand(seed);
    close(fp);
}

void InferenceRate::setInitialPartitionings(int forcedInput, bool initPart, char *initPartFile, bool multilevel, int *multilevelPart, int higherLevelNumberOfVertices) {
	int i, *j=NULL, k, r, l;

	if (!multilevel) {
		setForcedInputSize(forcedInput);
		if (initPart == false) {
			// random, memory valid but different amount of vertices per partition
			// force input layer (image) to be in the same device/partition
			/*int p = 0;
			i = 0;
			while (i < getForcedInputSize()) {
				if (validArray[p] < target->getMemory(p)) {
					setVertexInitialPartitionings(i, p);
					validArray[p] += sourceG->getMemory(i);
				} else {
					p++;	
				}
				i++;
			}	
			seed_rng();
			//int shared;
			for (i = getForcedInputSize(); i < getNumberOfVertices(); i++) {
				r = rand();

				// finds the layer i belongs
				int layer = 0, shared = 0;
				for (int j = 0; j < sourceG->getNumberOfLayers(); j++) {
					if (layer == sourceG->getNumberOfLayers() - 1)
						break;
					if (i < sourceG->getLayerInitialPos(j + 1)) {
						break;
					} else {
						layer++;
					}
				}
				//cout << "\n layer: " << layer;
				if (sharedMemoryPerPartition[r % getNumberOfPartitions()][layer] == false) {
					//sharedMemoryPerPartition[r % getNumberOfPartitions()][layer] = true;
					shared = sourceG->getSharedParam(layer);
					//cout << " sMPP: false"; 
				}
				//else
					//cout << " sMPP: true";
				//cout << " shared: " << shared;
				if (validArray[r % getNumberOfPartitions()] + sourceG->getMemory(i) + shared < target->getMemory(r % getNumberOfPartitions())) {
					if (sharedMemoryPerPartition[r % getNumberOfPartitions()][layer] == false) {
						sharedMemoryPerPartition[r % getNumberOfPartitions()][layer] = true;
					}
					setVertexInitialPartitionings(i, r % getNumberOfPartitions());
					validArray[r % getNumberOfPartitions()] += sourceG->getMemory(i) + shared;
					//cout << "\n i: "<< i << " v[" << r % getNumberOfPartitions() << "]: " << validArray[r % getNumberOfPartitions()];
				} else {
					bool set = false;
					int setp;
					if (r % getNumberOfPartitions() == getNumberOfPartitions() - 1) {
						setp = - r % getNumberOfPartitions();
					} else {
						setp = 1;
					}
					int count = 0;
					while(!set) {
						if (r % getNumberOfPartitions() + setp == getNumberOfPartitions()) {
							setp = - r % getNumberOfPartitions();
							count++;
							if (count == getNumberOfPartitions()) {
								break;
							}
						}
						if (validArray[r % getNumberOfPartitions() + setp] + sourceG->getMemory(i) + shared < target->getMemory(r % getNumberOfPartitions() + setp)) {
							setVertexInitialPartitionings(i, r % getNumberOfPartitions() + setp);
							validArray[r % getNumberOfPartitions() + setp] += sourceG->getMemory(i) + shared;
							set = true;
						} else {
							setp++;
						}
					}
					if (count == getNumberOfPartitions()) {
						cout << "\n No valid initial partitioning was found! \n\n";
						exit(1);
					}
					//cout << "\n i: "<< i << " v[" << r % getNumberOfPartitions() + setp << "]: " << validArray[r % getNumberOfPartitions() + setp];
				}
			}
		}*/

			// iRgreedy (for original graph) (did not produce valid partitionins even for 2 partitions)
			/*for (int l = 0; l < sourceG->getNumberOfLayers(); l++) {
				int supBound = sourceG->getLayerInitialPos(l + 1);
				if (l == sourceG->getNumberOfLayers() - 1) {
					supBound = sourceG->getNumberOfVertices();
				}
				int numberOfAssinedVerticesPerLayer = (supBound - sourceG->getLayerInitialPos(l)) / getNumberOfPartitions();

				// finds the layers v belongs
				bool shrdppv[sourceG->getNumberOfLayers()];
				int shared = 0;
				sourceG->getSharedParamArray(sourceG->getLayerInitialPos(l), shrdppv);

				for (int p = 0; p < getNumberOfPartitions(); p++) {
					int nOAVPLreal = numberOfAssinedVerticesPerLayer;
					int startIdx = sourceG->getLayerInitialPos(l);
					int endIdx = 0;

					if (p == getNumberOfPartitions() - 1 && ((supBound - sourceG->getLayerInitialPos(l)) % getNumberOfPartitions()) != 0) {
						endIdx = (supBound - sourceG->getLayerInitialPos(l)) % getNumberOfPartitions();
					}
				
					for (int v = 0 + p * nOAVPLreal + startIdx; v < nOAVPLreal + p * nOAVPLreal + startIdx + endIdx; v++) {
						if (shrdppv[l] == true && sharedMemoryPerPartition[p][l] == false) {
							sharedMemoryPerPartition[p][l] = true;
							shared += sourceG->getSharedParam(l);
						}

						if (validArray[p] + sourceG->getMemory(v) + shared < target->getMemory(p)) {
							setVertexInitialPartitionings(v, p);
							validArray[p] += sourceG->getMemory(v) + shared;
							cout << "\n v: "<< v << " p[" << p << "]: " << validArray[p];	
						} else {
							int pAlt = p, i = 0;
							while (i < getNumberOfPartitions()) {
								if (validArray[pAlt] + sourceG->getMemory(v) + shared < target->getMemory(pAlt)) {
									setVertexInitialPartitionings(v, pAlt);
									validArray[pAlt] += sourceG->getMemory(v) + shared;
									cout << "\n v: "<< v << " pAlt[" << pAlt << "]: " << validArray[pAlt];
									break;
								} else {
									i++;
									pAlt++;
									if (pAlt > getNumberOfPartitions() - 1) {
										pAlt = 0;
									}
								}
							}
						}
					}
				}
			}*/

			// iRgreedy2 (for constrained devices and original graph)
			/*int numberOfTheFirstSharedLayers = 5, lay[numberOfTheFirstSharedLayers] = {6, 5, 7, 3, 1}, p = 0;

			for (int k = 0; k < numberOfTheFirstSharedLayers; k++) {
				//cout << "\n lay[k]: " << lay[k];
				// finds the layers v belongs
				bool shrdppv[sourceG->getNumberOfLayers()];
				sourceG->getSharedParamArray(sourceG->getLayerInitialPos(lay[k]), shrdppv);

				for (int v = sourceG->getLayerInitialPos(lay[k]); v < sourceG->getLayerInitialPos(lay[k] + 1); v++) {
					int shared = 0;
					if (shrdppv[lay[k]] == true && sharedMemoryPerPartition[p][lay[k]] == false) {
						shared += sourceG->getSharedParam(lay[k]);
					}

					if (validArray[p] + sourceG->getMemory(v) + shared < target->getMemory(p)) {
						sharedMemoryPerPartition[p][lay[k]] = true;
						setVertexInitialPartitionings(v, p);
						validArray[p] += sourceG->getMemory(v) + shared;
						//cout << "\n v: "<< v << " p[" << p << "]: " << validArray[p];	
					} else {
						int pAlt = p, i = 0;
						while (i < getNumberOfPartitions()) {
							int shared = 0;
							if (shrdppv[lay[k]] == true && sharedMemoryPerPartition[pAlt][lay[k]] == false) {
								shared += sourceG->getSharedParam(lay[k]);
							}

							if (validArray[pAlt] + sourceG->getMemory(v) + shared < target->getMemory(pAlt)) {
								sharedMemoryPerPartition[pAlt][lay[k]] = true;
								setVertexInitialPartitionings(v, pAlt);
								validArray[pAlt] += sourceG->getMemory(v) + shared;
								//cout << "\n v: "<< v << " pAlt[" << pAlt << "]: " << validArray[pAlt];
								p = pAlt;
								break;
							} else {
								i++;
								pAlt++;
								if (pAlt > getNumberOfPartitions() - 1) {
									pAlt = 0;
								}
							}
						}
					}
				}
			}
			for (int l = 0; l < sourceG->getNumberOfLayers(); l++) {
				if (l == 6 || l == 5 || l == 7 || l == 3 || l == 1) continue;
				int supBound = sourceG->getLayerInitialPos(l + 1);
				if (l == sourceG->getNumberOfLayers() - 1) {
					supBound = sourceG->getNumberOfVertices();
				}
				int numberOfAssinedVerticesPerLayer = (supBound - sourceG->getLayerInitialPos(l)) / getNumberOfPartitions();

				// finds the layers v belongs
				bool shrdppv[sourceG->getNumberOfLayers()];
				sourceG->getSharedParamArray(sourceG->getLayerInitialPos(l), shrdppv);

				for (int p = 0; p < getNumberOfPartitions(); p++) {
					int nOAVPLreal = numberOfAssinedVerticesPerLayer;
					int startIdx = sourceG->getLayerInitialPos(l);
					int endIdx = 0;

					if (p == getNumberOfPartitions() - 1 && ((supBound - sourceG->getLayerInitialPos(l)) % getNumberOfPartitions()) != 0) {
						endIdx = (supBound - sourceG->getLayerInitialPos(l)) % getNumberOfPartitions();
					}
				
					for (int v = 0 + p * nOAVPLreal + startIdx; v < nOAVPLreal + p * nOAVPLreal + startIdx + endIdx; v++) {
						int shared = 0;
						if (shrdppv[l] == true && sharedMemoryPerPartition[p][l] == false) {
							shared += sourceG->getSharedParam(l);
						}

						if (validArray[p] + sourceG->getMemory(v) + shared < target->getMemory(p)) {
							sharedMemoryPerPartition[p][l] = true;
							setVertexInitialPartitionings(v, p);
							validArray[p] += sourceG->getMemory(v) + shared;
							//cout << "\n v: "<< v << " p[" << p << "]: " << validArray[p];	
						} else {
							int pAlt = p, i = 0;
							while (i < getNumberOfPartitions()) {
								int shared = 0;
								if (shrdppv[l] == true && sharedMemoryPerPartition[pAlt][l] == false) {
									shared += sourceG->getSharedParam(l);
								}
								if (validArray[pAlt] + sourceG->getMemory(v) + shared < target->getMemory(pAlt)) {
									sharedMemoryPerPartition[pAlt][l] = true;
									setVertexInitialPartitionings(v, pAlt);
									validArray[pAlt] += sourceG->getMemory(v) + shared;
									//cout << "\n v: "<< v << " pAlt[" << pAlt << "]: " << validArray[pAlt];
									break;
								} else {
									i++;
									pAlt++;
									if (pAlt > getNumberOfPartitions() - 1) {
										pAlt = 0;
									}
								}
							}
						}
					}
				}
			}*/

			// best fit
			//seed_rng();
			for (int i = 0; i < getNumberOfVertices(); i++) {

				// finds the layers i belongs
				bool shrdppv[sourceG->getNumberOfLayers()];
				int shared = 0;
				sourceG->getSharedParamArray(i, shrdppv);

				int part = 0;
				int minRemainder = 2147483647;
				for (int p = 0; p < getNumberOfPartitions(); p++) {

					// find the correct number of shared parameters according to the parameters that the partition already contain
					shared = 0;
					for (l = 0; l < sourceG->getNumberOfLayers(); l++) {
						if (shrdppv[l] == true && sharedMemoryPerPartition[p][l] == false) {
							shared += sourceG->getSharedParam(l);
						}
					}

					if (validArray[p] + sourceG->getMemory(i) + shared < target->getMemory(p)) {
						if (target->getMemory(p) - validArray[p] - sourceG->getMemory(i) - shared < minRemainder) {
							minRemainder = target->getMemory(p) - validArray[p] - sourceG->getMemory(i) - shared; // < minRemainder;
							part = p;

						}
					} /*else if (p == getNumberOfPartitions() - 1) {
						cout << "\n No valid initial partitioning was found! \n\n";
						//exit(1);
					}*/
				}

				// find the correct number of shared parameters according to the parameters that the partition already contain
				shared = 0;
				for (l = 0; l < sourceG->getNumberOfLayers(); l++) {
					if (shrdppv[l] == true && sharedMemoryPerPartition[part][l] == false) {
						shared += sourceG->getSharedParam(l);
					}
				}

				if (validArray[part] + sourceG->getMemory(i) + shared < target->getMemory(part)) {				
					for (l = 0; l < sourceG->getNumberOfLayers(); l++) {
						if (shrdppv[l] == true && sharedMemoryPerPartition[part][l] == false) {
							sharedMemoryPerPartition[part][l] = true;
						}
					}
					setVertexInitialPartitionings(i, part);
					validArray[part] += sourceG->getMemory(i) + shared;
					//cout << "\n i: "<< i << " v[" << part << "]: " << validArray[part];					
				}
			}
		} else if (initPart) {
				// input a specific initial partitioning
				FILE *fp;
				fp = fopen(initPartFile, "r");
				if(fp == NULL){
					cout << "initPart.txt file could not be open! \n";
					exit(EXIT_FAILURE);
				}

				for (i = 0; i < getNumberOfVertices(); i++) {
					l = fscanf(fp, "%d ", &k);
					setVertexInitialPartitionings(i, k);
				}
				fclose(fp);
				fp=NULL;
		}

			// worst fit
			//seed_rng();
			/*for (int i = 0; i < getNumberOfVertices(); i++) {

				// finds the layers i belongs
				bool shrdppv[sourceG->getNumberOfLayers()];
				int shared = 0;
				sourceG->getSharedParamArray(i, shrdppv);

				int part = 0;
				int maxRemainder = 0;
				for (int p = 0; p < getNumberOfPartitions(); p++) {

					// find the correct number of shared parameters according to the parameters that the partition already contain
					shared = 0;
					for (l = 0; l < sourceG->getNumberOfLayers(); l++) {
						if (shrdppv[l] == true && sharedMemoryPerPartition[p][l] == false) {
							shared += sourceG->getSharedParam(l);
						}
					}

					if (validArray[p] + sourceG->getMemory(i) + shared < target->getMemory(p)) {
						if (target->getMemory(p) - validArray[p] - sourceG->getMemory(i) - shared > maxRemainder) {
							maxRemainder = target->getMemory(p) - validArray[p] - sourceG->getMemory(i) - shared; // < minRemainder;
							part = p;

						}
					} /*else if (p == getNumberOfPartitions() - 1) {
						cout << "\n No valid initial partitioning was found! \n\n";
						//exit(1);
					}*/
				/*}

				// find the correct number of shared parameters according to the parameters that the partition already contain
				shared = 0;
				for (l = 0; l < sourceG->getNumberOfLayers(); l++) {
					if (shrdppv[l] == true && sharedMemoryPerPartition[part][l] == false) {
						shared += sourceG->getSharedParam(l);
					}
				}

				if (validArray[part] + sourceG->getMemory(i) + shared < target->getMemory(part)) {				
					for (l = 0; l < sourceG->getNumberOfLayers(); l++) {
						if (shrdppv[l] == true && sharedMemoryPerPartition[part][l] == false) {
							sharedMemoryPerPartition[part][l] = true;
						}
					}
					setVertexInitialPartitionings(i, part);
					validArray[part] += sourceG->getMemory(i) + shared;
					//cout << "\n i: "<< i << " v[" << part << "]: " << validArray[part];					
				}
			}*/
		//}
	} else {
		for (int j = 0; j < getNumberOfVertices(); j++) {	
			for (int i = 0; i < higherLevelNumberOfVertices; i++) {
				if (sourceG->getMap(j) == i) {
					setVertexInitialPartitionings(j, multilevelPart[i]);
				}
			}
		}

		int lockedArraySize = 0;
		for (int j = 0; j < getNumberOfVertices(); j++) {
			bool locked = true;
			for (links nodeAdj = sourceG->getAdjOfVertex(j); nodeAdj != NULL; nodeAdj = nodeAdj->next) {
				//cout << "\nj: " << j << ", adj: " << nodeAdj->w << ", p[j]: " << getCurrentPartitioning(j) << ", p[adj]: " << getCurrentPartitioning(nodeAdj->w);
				if (getCurrentPartitioning(j) != getCurrentPartitioning(nodeAdj->w) && nodeAdj->w > j) {
					locked = false;
					break;
				}
			}
			if (locked == true) {
				setLockedArray(lockedArraySize, j);
				lockedArraySize++;
			}
		}
		setLockedArraySize(lockedArraySize);

		/*for (int j = 0; j < lockedArraySize; j++) {
			cout << "\ni: " << getLockedArray(j);
		}*/
	}
}

/* Returns the cost of partition the graph with partitioning p. */
/* For InferenceRate, the cost is the minimum value among the inference rate of each device and of each connection between all the devices (except when it is zero, i.e., when the devices cannot communicate with each other) */
//double InferenceRate::ComputeCost(Partitioning_t& p) {
double InferenceRate::computeCost(const int *partitioning, bool openmp, bool nonredundantEdges) {
	double finalInferenceRate = 2147483647;

	if (openmp) {
		for (int i = 0; i < getNumberOfPartitions(); i++) {
			partitionComputationalWeightPerThread[omp_get_thread_num()][i] = 0;		
		}

		for (int v = 0; v < getNumberOfPartitions(); ++v) {
			for (int i = 0; i < getNumberOfPartitions(); i++) {
				connectionMatrixInferenceRatePerThread[omp_get_thread_num()][v][i] = 0;
			}
		}

		// calculate the computational power needed by each partition
		for (int i = 0; i < getNumberOfVertices(); i++) {
			partitionComputationalWeightPerThread[omp_get_thread_num()][partitioning[i]] += sourceG->getVertexWeight(i);
		}

		// calculate the inference rate of each device
		for (int i = 0; i < getNumberOfPartitions(); i++) {
			if (partitionComputationalWeightPerThread[omp_get_thread_num()][i] != 0) {
				//cout << "\n i: " << i << ", getAssigned: " << target->getAssigned(i) << ", getCompW[assigned]: " << target->getComputationalWeight(target->getAssigned(i));
				deviceInferenceRatePerThread[omp_get_thread_num()][i] = target->getComputationalWeight(target->getAssigned(i)) / partitionComputationalWeightPerThread[omp_get_thread_num()][i];
				//cout << "\n NeededCompPower[" << i << "]: " << partitionComputationalWeight[i] << ", devInfRate[" << i << "]: " << deviceInferenceRate[i];
			} else {
				deviceInferenceRatePerThread[omp_get_thread_num()][i] = 2147483647;
			}
		}

		// calculate the computational power needed by each connection between each device
		bool inserted;

		delete partitionGraphPerThread[omp_get_thread_num()];
		// build partitionGraph based on SourceG (input graph)
		partitionGraphPerThread[omp_get_thread_num()] = new SourceGraph(getNumberOfPartitions(), getNumberOfPartitions());

		for (int i = 0; i < getNumberOfVertices(); i++) {
			for (links nodeAdj = sourceG->getAdjOfVertex(i); nodeAdj != NULL; nodeAdj = nodeAdj->next) {
				if (partitioning[i] != partitioning[nodeAdj->w] && nodeAdj->w > i) {
					inserted = partitionGraphPerThread[omp_get_thread_num()]->insertArcSrcMlvlPartitionGraph(partitioning[i], partitioning[nodeAdj->w], nodeAdj->edgeWeight, i, nodeAdj->sourceMlvl);
					if (inserted) {
						//currentCost += nodeAdj->edgeWeight;
						connectionMatrixInferenceRatePerThread[omp_get_thread_num()][partitioning[i]][partitioning[nodeAdj->w]] += nodeAdj->edgeWeight;
						connectionMatrixInferenceRatePerThread[omp_get_thread_num()][partitioning[nodeAdj->w]][partitioning[i]] += nodeAdj->edgeWeight;
						//cout << "\n i: " << i << " nodeAdj: " << nodeAdj->w << " eW: " << nodeAdj->edgeWeight << " redN: " << nodeAdj->redundantNeurons << " connMatrixInfRate[p[i]][p[nodeAdj]]: " << connectionMatrixInferenceRate[partitioning[i]][partitioning[nodeAdj->w]];
					}
				}
			}
		}

		// calculate the inference rate of each connection between each device
		for (int v = 0; v < getNumberOfPartitions(); ++v) {
			for (int i = 0; i < getNumberOfPartitions(); i++) {
				if (connectionMatrixInferenceRatePerThread[omp_get_thread_num()][v][i] != 0) {
					connectionMatrixInferenceRatePerThread[omp_get_thread_num()][v][i] = target->getConnectionWeight(v, i) / connectionMatrixInferenceRatePerThread[omp_get_thread_num()][v][i];
				} else {
					connectionMatrixInferenceRatePerThread[omp_get_thread_num()][v][i] = 2147483647;
				}
				//cout << "\n v: " << v << ", i: " << i << ", getConnW: " << target->getConnectionWeight(v,i) << ", connMatrixInfRate[v][i]: " << connectionMatrixInferenceRate[v][i];
			}
		}

		// choose the minimum value among the inference rate of each device 
		for (int i = 0; i < getNumberOfPartitions(); i++) {
			if (finalInferenceRate > deviceInferenceRatePerThread[omp_get_thread_num()][i]) {
				finalInferenceRate = deviceInferenceRatePerThread[omp_get_thread_num()][i];
			}
		}	
	
		// choose the minimum value among each connection between all the devices (except when it is zero, i.e., when the devices cannot communicate with each other)
		for (int v = 0; v < getNumberOfPartitions(); ++v) {
			for (int i = 0; i < getNumberOfPartitions(); i++) {
				if (i != v) {
					if (finalInferenceRate > connectionMatrixInferenceRatePerThread[omp_get_thread_num()][v][i]) {
						finalInferenceRate = connectionMatrixInferenceRatePerThread[omp_get_thread_num()][v][i];
					}
				}
			}
		}
	} else {
		for (int i = 0; i < getNumberOfPartitions(); i++) {
			partitionComputationalWeight[i] = 0;		
		}

		for (int v = 0; v < getNumberOfPartitions(); ++v) {
			for (int i = 0; i < getNumberOfPartitions(); i++) {
				connectionMatrixInferenceRate[v][i] = 0;
			}
		}

		// calculate the computational power needed by each partition
		for (int i = 0; i < getNumberOfVertices(); i++) {
			partitionComputationalWeight[partitioning[i]] += sourceG->getVertexWeight(i);
		}

		// calculate the inference rate of each device
		for (int i = 0; i < getNumberOfPartitions(); i++) {
			if (partitionComputationalWeight[i] != 0) {
				//cout << "\n i: " << i << ", getAssigned: " << target->getAssigned(i) << ", getCompW[assigned]: " << target->getComputationalWeight(target->getAssigned(i));
				deviceInferenceRate[i] = target->getComputationalWeight(target->getAssigned(i)) / partitionComputationalWeight[i];
				//cout << "\n NeededCompPower[" << i << "]: " << partitionComputationalWeight[i] << ", devInfRate[" << i << "]: " << deviceInferenceRate[i];
			} else {
				deviceInferenceRate[i] = 2147483647;
			}
		}

		// calculate the computational power needed by each connection between each device
		bool inserted;

		delete partitionGraph;
		// build partitionGraph based on SourceG (input graph)
		partitionGraph = new SourceGraph(getNumberOfPartitions(), getNumberOfPartitions());

		for (int i = 0; i < getNumberOfVertices(); i++) {
			for (links nodeAdj = sourceG->getAdjOfVertex(i); nodeAdj != NULL; nodeAdj = nodeAdj->next) {
				if (partitioning[i] != partitioning[nodeAdj->w] && nodeAdj->w > i) {
					inserted = partitionGraph->insertArcSrcMlvlPartitionGraph(partitioning[i], partitioning[nodeAdj->w], nodeAdj->edgeWeight, /*nodeAdj->redundantNeurons,*/ i, nodeAdj->sourceMlvl);
					if (inserted) {
						//currentCost += nodeAdj->edgeWeight;
						connectionMatrixInferenceRate[partitioning[i]][partitioning[nodeAdj->w]] += nodeAdj->edgeWeight;
						connectionMatrixInferenceRate[partitioning[nodeAdj->w]][partitioning[i]] += nodeAdj->edgeWeight;
						//cout << "\n i: " << i << " nodeAdj: " << nodeAdj->w << " eW: " << nodeAdj->edgeWeight << " redN: " << nodeAdj->redundantNeurons << " connMatrixInfRate[p[i]][p[nodeAdj]]: " << connectionMatrixInferenceRate[partitioning[i]][partitioning[nodeAdj->w]];
					}
				}
			}
		}

		// calculate the inference rate of each connection between each device
		for (int v = 0; v < getNumberOfPartitions(); ++v) {
			for (int i = 0; i < getNumberOfPartitions(); i++) {
				if (connectionMatrixInferenceRate[v][i] != 0) {
					connectionMatrixInferenceRate[v][i] = target->getConnectionWeight(v, i) / connectionMatrixInferenceRate[v][i];
				} else {
					connectionMatrixInferenceRate[v][i] = 2147483647;
				}
				//cout << "\n v: " << v << ", i: " << i << ", getConnW: " << target->getConnectionWeight(v,i) << ", connMatrixInfRate[v][i]: " << connectionMatrixInferenceRate[v][i];
			}
		}

		// choose the minimum value among the inference rate of each device 
		for (int i = 0; i < getNumberOfPartitions(); i++) {
			if (finalInferenceRate > deviceInferenceRate[i]) {
				finalInferenceRate = deviceInferenceRate[i];
			}
		}	
	
		// choose the minimum value among each connection between all the devices (except when it is zero, i.e., when the devices cannot communicate with each other)
		for (int v = 0; v < getNumberOfPartitions(); ++v) {
			for (int i = 0; i < getNumberOfPartitions(); i++) {
				if (i != v) {
					if (finalInferenceRate > connectionMatrixInferenceRate[v][i]) {
						finalInferenceRate = connectionMatrixInferenceRate[v][i];
					}
				}
			}
		}
	}

	return finalInferenceRate;
}

void InferenceRate::computeRedundantMemoryPerPartition(const int *p) {
	// Calculate amount of memory needed for each partition	
	for (int i = 0; i < getNumberOfPartitions(); i++) {
		validArray[i] = 0;
	}
	for (int i = 0; i < sourceG->getNumberOfVertices(); i++) {
		validArray[p[i]] += sourceG->getMemory(i);
	}
}

void InferenceRate::computeNonredundantMemoryPerPartition(const int *p) {
	int i, j;

	for (i = 0; i < getNumberOfPartitions(); i++) {
		validArrayPerThread[omp_get_thread_num()][i] = 0;
		for (int j = 0; j < sourceG->getNumberOfLayers(); j++) {
			sharedMemoryPerPartitionPerThread[omp_get_thread_num()][i][j] = false;
		}
	}

	// Calculate amount of memory needed for each partition
	for (i = 0; i < sourceG->getNumberOfVertices(); i++) {
		validArrayPerThread[omp_get_thread_num()][p[i]] += sourceG->getMemory(i);
		// Sequential code (initially j = 0 above and sharedMemPerPart[p[i]][j - 1] below
		/*if (i >= sourceG->getLayerInitialPos(j)) {
			j++;
			if (j >= sourceG->getNumberOfLayers()) {
				j = sourceG->getNumberOfLayers() - 1;
			}
		}*/

		// finds the layers i belongs
		bool shrdppv[sourceG->getNumberOfLayers()];
		sourceG->getSharedParamArray(i, shrdppv);		
		
		for (int j = 0; j < sourceG->getNumberOfLayers(); j++) {
			if (shrdppv[j] == true) {
				sharedMemoryPerPartitionPerThread[omp_get_thread_num()][p[i]][j] = true;
			}
		}
	}
	for (i = 0; i < getNumberOfPartitions(); i++) {
		//cout << "\nThread[" << omp_get_thread_num() << "]: i: " << i ;
		for (int j = 0; j < sourceG->getNumberOfLayers(); j++) {
			if (sharedMemoryPerPartitionPerThread[omp_get_thread_num()][i][j] == true) {
				validArrayPerThread[omp_get_thread_num()][i] += sourceG->getSharedParam(j);
			}
		}
	}
}

bool InferenceRate::partitionsFitDevices(const int *p, int *sourceMem, bool openmp) {
	int a, b;

	mergeSortWithIndexes(sourceMem, 1, getNumberOfPartitions(), validIndexes);

	if (openmp) {
		// nonnaive implementation: sort validArray and targetMem in descending order
		// this makes the sorting not stable; to have a stable sort comment this block and use Source_graph->V/2 - 1 - a (or b) in FindMaxReductionInCost()
		for (a = 0; a < getNumberOfPartitions()/2; a++) {
			b = getNumberOfPartitions() - a - 1;

			int sortaux = sourceMem[a];
			sourceMem[a] = sourceMem[b];
			sourceMem[b] = sortaux;

			sortaux = validIndexesPerThread[omp_get_thread_num()][a];
			validIndexesPerThread[omp_get_thread_num()][a] = validIndexesPerThread[omp_get_thread_num()][b];
			validIndexesPerThread[omp_get_thread_num()][b] = sortaux;
		}

		for (a = 0; a < getNumberOfPartitions(); a++) {
			if (sourceMem[a] > sortedTargetMem[a]) {
				free(sourceMem);
				return false;
			} else {
				target->setAssignedPerThread(a, targetIndexes[a], omp_get_thread_num());
			}
		}
	} else {
		// nonnaive implementation: sort validArray and targetMem in descending order
		// this makes the sorting not stable; to have a stable sort comment this block and use Source_graph->V/2 - 1 - a (or b) in FindMaxReductionInCost()
		for (a = 0; a < getNumberOfPartitions()/2; a++) {
			b = getNumberOfPartitions() - a - 1;

			int sortaux = sourceMem[a];
			sourceMem[a] = sourceMem[b];
			sourceMem[b] = sortaux;

			sortaux = validIndexes[a];
			validIndexes[a] = validIndexes[b];
			validIndexes[b] = sortaux;
		}

		for (a = 0; a < getNumberOfPartitions(); a++) {
			if (sourceMem[a] > sortedTargetMem[a]) {
				free(sourceMem);
				return false;
			} else {
				target->setAssigned(a, targetIndexes[a]);
			}
		}
	}

	free(sourceMem);
	return true;
}

/* Returns true if the partitioning is valid, false otherwise. */
bool InferenceRate::validPartitioning(const int *p) {

	if (sourceG->getEnableMemory() == 0) {
		return true;
	} else if (sourceG->getEnableMemory() == 1) {
		// Calculate amount of memory needed for each partition
		computeNonredundantMemoryPerPartition(p);

		//static int *sourceMem = (int *) malloc(getNumberOfPartitions() * sizeof(int));
		int *sourceMem = (int *) malloc(getNumberOfPartitions() * sizeof(int));
		if(sourceMem == NULL) {
			cout << "\n sourceMem could not be allocated! \n";
			exit(EXIT_FAILURE);	
		}
		for (int a = 0; a < getNumberOfPartitions(); a++) {
			sourceMem[a] = validArrayPerThread[omp_get_thread_num()][a];
		}
		// Check if every partition fits to devices
		return partitionsFitDevices(p, sourceMem, false);
	}
}

void InferenceRate::computeDiffNodeMemoryPerPartition(int *partitioning, 
								   				   unsigned nodeOrK,
												   unsigned originalNodeOrKPartition,
												   int *sourceMem) {
	int i, layer = 0;
	bool sumSharedMem = true, subtractSharedMem = true;

	// vertice (node or k) memory
	sourceMem[partitioning[nodeOrK]] += sourceG->getMemory(nodeOrK);
	sourceMem[originalNodeOrKPartition] -= sourceG->getMemory(nodeOrK);

	// finds the layer node or k belongs
	for (i = 0; i < sourceG->getNumberOfLayers(); i++) {
		if (layer == sourceG->getNumberOfLayers() - 1)
			break;
		if (nodeOrK < sourceG->getLayerInitialPos(i + 1)) {
			break;
		} else {
			layer++;
		}
	}

	int last;
	if (layer >= sourceG->getNumberOfLayers() - 1) {
		last = sourceG->getNumberOfVertices();
	} else {
		last = sourceG->getLayerInitialPos(layer + 1);
	}
	// searches if new node or k partition must sum shared memory
	for (i = sourceG->getLayerInitialPos(layer); i < last; i++) {
		if (i == nodeOrK) continue;
		if (partitioning[i] == partitioning[nodeOrK]) {
			sumSharedMem = false;
			break;
		}
	}
	if (sumSharedMem) {
		sourceMem[partitioning[nodeOrK]] += sourceG->getSharedParam(layer);
	}

	// searches if old node or k partition must subtract shared memory
	for (i = sourceG->getLayerInitialPos(layer); i < last; i++) {
		if (i == nodeOrK) continue;
		if (partitioning[i] == originalNodeOrKPartition) {
			subtractSharedMem = false;
			break;
		}
	}
	if (subtractSharedMem) {
		sourceMem[originalNodeOrKPartition] -= sourceG->getSharedParam(layer);
	}
}

void InferenceRate::computeDiffMemoryPerPartition(int *partitioning, 
								   				 unsigned node, 
								   				 unsigned k, 
								   				 unsigned originalNodePartition, 
								   				 unsigned original_k_Partition,
												 int *sourceMem,
												 bool singleMoveOrSwap) {

	// compute for node
	computeDiffNodeMemoryPerPartition(partitioning, node, originalNodePartition, sourceMem);

	// compute for k in case of swaps	
	if (singleMoveOrSwap) {
		computeDiffNodeMemoryPerPartition(partitioning, k, original_k_Partition, sourceMem);
	}
}

/* Returns true if the partitioning is valid, false otherwise. */
bool InferenceRate::diffValidPartitioning(int *partitioning,
								   	   unsigned node, 
								   	   unsigned k, 
								   	   unsigned originalNodePartition, 
								   	   unsigned original_k_Partition,
					bool singleOrSwap) {
	//SourceGraph *srcG = getSourceGraph();

	if (sourceG->getEnableMemory() == 0) {
		return true;
	} else if (sourceG->getEnableMemory() == 1) {

		//static int *sourceMem = (int *) malloc(getNumberOfPartitions() * sizeof(int));
		int *sourceMem = (int *) malloc(getNumberOfPartitions() * sizeof(int));
		if(sourceMem == NULL) {
			cout << "\n sourceMem could not be allocated! \n";
			exit(EXIT_FAILURE);	
		}
		for (int a = 0; a < getNumberOfPartitions(); a++) {
			sourceMem[a] = validArray[a];
		}

		// Calculate amount of memory needed for each partition
		computeDiffMemoryPerPartition(partitioning, node, k, originalNodePartition, original_k_Partition, sourceMem, singleOrSwap);

		// Check if every partition fits to devices
		return partitionsFitDevices(partitioning, sourceMem, true);
	}
}

int InferenceRate::getValidArray(int i) const {
	return validArray[i];
}

int InferenceRate::getValidArrayPerThread(int t, int i) const {
	return validArrayPerThread[t][i];
}

int InferenceRate::getValidIndex(int i) const {
	return validIndexes[i];
}

void InferenceRate::mergeSortWithIndexes(int *A, int p, int r, int *indexes){
	int j, **Ai=NULL;

	Ai = (int **) calloc(r, sizeof(int *));
	if (Ai == NULL) {
		printf("Ai could not be allocated!\n");
		exit(EXIT_FAILURE);
	}
	for (j = 0; j < r; j++) {
		Ai[j] = (int *) calloc(2, sizeof(int));
		if (Ai[j] == NULL) {
			printf("Ai[%d] could not be allocated!\n", j);
			exit(EXIT_FAILURE);
		}
	}

	for (j = 0; j < r; j++) {
		Ai[j][0] = j;
		Ai[j][1] = A[j];
	}

	/*for (j = p; j <= r; j++) {
		printf("\n%d %d", Ai[j - 1][0], Ai[j - 1][1]);
	}
	printf("\n");*/

	mergeSort(Ai, p, r);

	for (j = p; j <= r; j++) {
		A[j - 1] = Ai[j - 1][1];
		indexes[j - 1] = Ai[j - 1][0];
	}

	/*for (j = p; j <= r; j++) {
		printf("\n%d %d", Ai[j - 1][0], Ai[j - 1][1]);
	}
	printf("\n");*/

	for (j = 0; j < r; j++) {
		free(Ai[j]);
		Ai[j] = NULL;
	}	
	free(Ai);
	Ai=NULL;
}

void InferenceRate::mergeSort(int **A, int p, int r) {
	int j, q;

	if (p < r) {
		//q = floor((p+r)/2);
		q = (p + r)/2;
		mergeSort(A, p, q);
		mergeSort(A, q + 1, r);
		merge(A, p, q, r);
	}
}

void InferenceRate::merge(int **A, int p, int q, int r) {
	int n1, n2, **L=NULL, **R=NULL, i, j, k;

	n1 = q - p + 1;
	n2 = r - q;

	L = (int **) calloc(n1 + 1, sizeof(int *));
	if (L == NULL) {
		printf("L could not be allocated!\n");
		exit(EXIT_FAILURE);
	}
	for (j = 0; j < n1 + 1; j++) {
		L[j] = (int *) calloc(2, sizeof(int));
		if (L[j] == NULL) {
			printf("L[%d] could not be allocated!\n", j);
			exit(EXIT_FAILURE);
		}
	}
	R = (int **) calloc(n2 + 1, sizeof(int *));
	if (R == NULL) {
		printf("R could not be allocated!\n");
		exit(EXIT_FAILURE);
	}
	for (j = 0; j < n2 + 1; j++) {
		R[j] = (int *) calloc(2, sizeof(int));
		if (R[j] == NULL) {
			printf("R[%d] could not be allocated!\n", j);
			exit(EXIT_FAILURE);
		}
	}

	for (i = 1; i < n1 + 1; i++) {
		L[i - 1][1] = A[p + i - 2][1];
		L[i - 1][0] = A[p + i - 2][0];
	}
	for (j = 1; j < n2 + 1; j++) {
		R[j - 1][1] = A[q + j - 1][1];
		R[j - 1][0] = A[q + j - 1][0];
	}

	L[n1][1] = 2147483647;
	L[n1][0] = -1;
	R[n2][1] = 2147483647;
	R[n2][0] = -1;

	i = 1;
	j = 1;

	for (k = p; k <= r; k++) {
		if (L[i - 1][1] <= R[j - 1][1]) {
			A[k - 1][1] = L[i - 1][1];
			A[k - 1][0] = L[i - 1][0];
			i++;
		} else {
			A[k - 1][1] = R[j - 1][1];
			A[k - 1][0] = R[j - 1][0];
			j++;
		}
	}

	for (j = 0; j < n1 + 1; j++) {
		free(L[j]);
		L[j] = NULL;
	}	
	free(L);
	L=NULL;

	for (j = 0; j < n2 + 1; j++) {
		free(R[j]);
		R[j] = NULL;
	}	
	free(R);
	R=NULL;
}

InferenceRate::~InferenceRate() {
	if (validArray != NULL) {
		free(validArray);
		validArray = NULL;
	}
    if (validArrayPerThread != NULL) {
            for (int i = 0; i < getNumberOfThreads(); i++) {
                    if (validArrayPerThread[i] != NULL) {
                            free(validArrayPerThread[i]);
                    }
            }
            free(validArrayPerThread);
            validArrayPerThread = NULL;
    }
	if (validIndexes != NULL) {
		free(validIndexes);
		validIndexes = NULL;
	}
	if (targetIndexes != NULL) {
		free(targetIndexes);
		targetIndexes = NULL;
	}
	if (sortedTargetMem != NULL) {
		free(sortedTargetMem);
		sortedTargetMem = NULL;
	}
	if (sharedMemoryPerPartition != NULL) {
		for (int i = 0; i < getNumberOfPartitions(); i++) {
			if (sharedMemoryPerPartition[i] != NULL) {
				free(sharedMemoryPerPartition[i]);
			}
		}
		free(sharedMemoryPerPartition);
		sharedMemoryPerPartition = NULL;
	}
	if (partitionComputationalWeight != NULL) {
		free(partitionComputationalWeight);
		partitionComputationalWeight = NULL;
	}
	if (deviceInferenceRate != NULL) {
		free(deviceInferenceRate);
		deviceInferenceRate = NULL;
	}
	if (connectionMatrixInferenceRate != NULL) {
		for (int i = 0; i < getNumberOfPartitions(); i++) {
			if (connectionMatrixInferenceRate[i] != NULL) {
				free(connectionMatrixInferenceRate[i]);
			}
		}
		free(connectionMatrixInferenceRate);
		connectionMatrixInferenceRate = NULL;
	}

	if (partitionComputationalWeightPerThread != NULL) {
		for (int i = 0; i < getNumberOfThreads(); i++) {
			if (partitionComputationalWeightPerThread[i] != NULL)
				free(partitionComputationalWeightPerThread[i]);
		}
		free(partitionComputationalWeightPerThread);
		partitionComputationalWeightPerThread = NULL;
	}
	if (deviceInferenceRatePerThread != NULL) {
		for (int i = 0; i < getNumberOfThreads(); i++) {
			if (deviceInferenceRatePerThread[i] != NULL)
				free(deviceInferenceRatePerThread[i]);
		}
		free(deviceInferenceRatePerThread);
		deviceInferenceRatePerThread = NULL;
	}
	if (connectionMatrixInferenceRatePerThread != NULL) {
		for (int t = 0; t < getNumberOfThreads(); t++) {
			for (int i = 0; i < getNumberOfPartitions(); i++) {
				if (connectionMatrixInferenceRatePerThread[t][i] != NULL) {
					free(connectionMatrixInferenceRatePerThread[t][i]);
				}
			}
			if (connectionMatrixInferenceRatePerThread[t] != NULL) {
				free(connectionMatrixInferenceRatePerThread[t]);
			}
		}
		free(connectionMatrixInferenceRatePerThread);
		connectionMatrixInferenceRatePerThread = NULL;
	}		
   if (sharedMemoryPerPartitionPerThread != NULL) {
            for (int t = 0; t < getNumberOfThreads(); t++) {
                    if (sharedMemoryPerPartitionPerThread[t] != NULL) {
                            for (int i = 0; i < getNumberOfPartitions(); i++) {
                                    if (sharedMemoryPerPartitionPerThread[t][i] != NULL) {
                                            free(sharedMemoryPerPartitionPerThread[t][i]);
                                    }
                            }
                            free(sharedMemoryPerPartitionPerThread[t]);
                            sharedMemoryPerPartitionPerThread[t] = NULL;
                    }
            }
            free(sharedMemoryPerPartitionPerThread);
            sharedMemoryPerPartitionPerThread = NULL;
    }
	delete partitionGraph;
	if (partitionGraphPerThread != NULL) {
		for (int i = 0; i < getNumberOfThreads(); i++) {
			if (partitionGraphPerThread[i] != NULL) {
				free(partitionGraphPerThread[i]);
			}
		}
		free(partitionGraphPerThread);
		partitionGraphPerThread = NULL;
	}
	if (validIndexesPerThread != NULL) {
		for (int i = 0; i < getNumberOfThreads(); i++) {
			if (validIndexesPerThread[i] != NULL) {
				free(validIndexesPerThread[i]);
			}
		}
		free(validIndexesPerThread);
		validIndexesPerThread = NULL;
	}
}
