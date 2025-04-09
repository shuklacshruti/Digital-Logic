#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VARIABLES 1024
#define MAX_GATES 1000
#define MAX_NAME_LENGTH 16
#define MAX_COUNT 64

typedef struct {
    char name[MAX_NAME_LENGTH];
    int value;
} Variable;

typedef struct {
    char type[MAX_NAME_LENGTH];
    int inputIndices[MAX_COUNT];
    int outputIndices[MAX_COUNT];
    int numInputs;
    int numOutputs;
    int size;
} Gate; 

Gate gates[MAX_GATES];
int numGates = 0;

Variable variables[MAX_VARIABLES];
int numInputs = 0, numOutputs = 0, numVariables = 0;

int dependencyGraph[MAX_GATES][MAX_GATES] = {0};

void evaluateGate(const char *type, int inputs[], int outputs[], int size);
int getVariableIndex(const char *name);
void resolveDependencies(void);
void printTruthTable(void);
int getValue(int index); 
void setValue(int index, int value);

int evaluateMultiplexer(int inputs[], int size) {
    int numDataInputs = 1 << size;
    int selectorValue = 0;

    for (int i = 0; i < size; i++) {
        selectorValue = (selectorValue << 1) | getValue(inputs[numDataInputs + i]);
    }

    return getValue(inputs[selectorValue]);
}

int getVariableIndex(const char *name) {
    if (strcmp(name, "1") == 0) {
        return -1;
    }
    if (strcmp(name, "0") == 0) {
        return -2; 
    }

    for (int i = 0; i < numVariables; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return i;
        }
    }

    if (numVariables < MAX_VARIABLES) {
        strcpy(variables[numVariables].name, name);
        variables[numVariables].value = 0;
        numVariables++;
        return numVariables - 1;
    }

    fprintf(stderr, "Error: Maximum number of variables reached.\n");
    exit(EXIT_FAILURE);
}

int getValue(int index) {
    if (index == -1) return 1; 
    if (index == -2) return 0; 
    return variables[index].value;
}

void setValue(int index, int value) {
    if (index >= 0) {
        variables[index].value = value;
    }
}

void evaluateGate(const char *type, int inputs[], int outputs[], int size) {
    if (strcmp(type, "AND") == 0) {
        int result = 1;
        for (int i = 0; i < 2; i++) {
            result &= getValue(inputs[i]);
        }
        setValue(outputs[0], result);
    } else if (strcmp(type, "OR") == 0) {
        int result = 0;
        for (int i = 0; i < 2; i++) {
            result |= getValue(inputs[i]);
        }
        setValue(outputs[0], result);
    } else if (strcmp(type, "NOT") == 0) {
        setValue(outputs[0], !getValue(inputs[0]));
    } else if (strcmp(type, "XOR") == 0) {
        setValue(outputs[0], getValue(inputs[0]) ^ getValue(inputs[1]));
    } else if (strcmp(type, "NAND") == 0) {
        int result = 1;
        for (int i = 0; i < 2; i++) {
            result &= getValue(inputs[i]);
        }
        setValue(outputs[0], !result);
    } else if (strcmp(type, "NOR") == 0) {
        int result = 0;
        for (int i = 0; i < 2; i++) {
            result |= getValue(inputs[i]);
        }
        setValue(outputs[0], !result);
    } else if (strcmp(type, "PASS") == 0) {
        setValue(outputs[0], getValue(inputs[0]));
    } else if (strcmp(type, "DECODER") == 0) {
        for (int i = 0; i < (1 << size); i++) {
            setValue(outputs[i], 0);  
        }
        
        int index = 0;
        for (int i = 0; i < size; i++) {
            index = (index << 1) | getValue(inputs[i]);
        }
        
        setValue(outputs[index], 1);
    } else if (strcmp(type, "MULTIPLEXER") == 0) {
        setValue(outputs[0], evaluateMultiplexer(inputs, size));
    }
}

void buildDependencyGraph() {
    for (int i = 0; i < numGates; i++) {
        for (int j = 0; j < gates[i].numInputs; j++) {
            int inputIndex = gates[i].inputIndices[j];
            for (int k = 0; k < numGates; k++) {
                for (int l = 0; l < gates[k].numOutputs; l++) {
                    if (gates[k].outputIndices[l] == inputIndex) {
                        dependencyGraph[k][i] = 1; // Gate k is a dependency of gate i
                        break;
                    }
                }
            }
        }
    }
}


 void resolveDependencies(void) {
    int inDegree[MAX_GATES] = {0};
    Gate sortedGates[MAX_GATES];
    int sortedIndex = 0;

    for (int i = 0; i < numGates; i++) {
        for (int j = 0; j < numGates; j++) {
            if (dependencyGraph[j][i]) {
                inDegree[i]++;
            }
        }
    }

    // topological sort
    while (sortedIndex < numGates) {
        int found = 0;
        for (int i = 0; i < numGates; i++) {
            if (inDegree[i] == 0) {
                sortedGates[sortedIndex++] = gates[i];
                inDegree[i] = -1; 
                
                for (int j = 0; j < numGates; j++) {
                    if (dependencyGraph[i][j]) {
                        inDegree[j]--;
                    }
                }
                found = 1;
                break;
            }
        }
        
        if (!found) {
            fprintf(stderr, "Error: Cycle detected in the circuit description.\n");
            exit(EXIT_FAILURE);
        }
    }
    
    memcpy(gates, sortedGates, sizeof(Gate) * numGates);
} 

void printTruthTable(void) {
    int numRows = 1 << numInputs;

    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numVariables; j++) {
            variables[j].value = 0;
        }

        for (int j = 0; j < numInputs; j++) {
            variables[j].value = (i >> (numInputs - j - 1)) & 1;
            printf("%d", variables[j].value);  
            if (j < numInputs - 1) {
                printf(" "); 
            }
        }
        printf(" | ");

        for (int g = 0; g < numGates; g++) {
            Gate gate = gates[g];
            int inputs[MAX_COUNT], outputs[MAX_COUNT];

            for (int k = 0; k < gate.numInputs; k++) {
                inputs[k] = gate.inputIndices[k];
            }
            for (int k = 0; k < gate.numOutputs; k++) {
                outputs[k] = gate.outputIndices[k];
            }

            evaluateGate(gate.type, inputs, outputs, gate.size);
        }

        // Print output values
        for (int j = 0; j < numOutputs; j++) {
            printf("%d", variables[numInputs + j].value);
            if (j < numOutputs - 1) {
                printf(" ");  
            }
        }

        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <circuit_description_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    char directive[MAX_NAME_LENGTH];

    // Read inputs, outputs, and gates into memory
    while (fscanf(file, " %15s", directive) == 1) {
        if (strcmp(directive, "INPUT") == 0) {
            fscanf(file, "%d", &numInputs);
            for (int i = 0; i < numInputs; i++) {
                char name[MAX_NAME_LENGTH];
                fscanf(file, " %15s", name);
                getVariableIndex(name);
            }
        } else if (strcmp(directive, "OUTPUT") == 0) {
            fscanf(file, "%d", &numOutputs);
            for (int i = 0; i < numOutputs; i++) {
                char name[MAX_NAME_LENGTH];
                fscanf(file, " %15s", name);
                getVariableIndex(name);
            }
        } else {
            int size = 0;
            if (strcmp(directive, "DECODER") == 0 || strcmp(directive, "MULTIPLEXER") == 0) {
                fscanf(file, "%d", &size);
            }

            int numInputsForGate = (strcmp(directive, "MULTIPLEXER") == 0) ? (1 << size) + size :
                                   (strcmp(directive, "DECODER") == 0) ? size :
                                   (strcmp(directive, "NOT") == 0 || strcmp(directive, "PASS") == 0) ? 1 : 2;

            int inputs[MAX_COUNT], outputs[MAX_COUNT];

            for (int i = 0; i < numInputsForGate; i++) {
                char inputName[MAX_NAME_LENGTH];
                fscanf(file, " %15s", inputName);
                inputs[i] = getVariableIndex(inputName);
            }

            int numOutputsForGate = (strcmp(directive, "DECODER") == 0) ? (1 << size) : 1;

            for (int i = 0; i < numOutputsForGate; i++) {
                char outputName[MAX_NAME_LENGTH];
                fscanf(file, " %15s", outputName);
                outputs[i] = getVariableIndex(outputName);
            }

            gates[numGates] = (Gate){.type = "", .size = size, .numInputs = numInputsForGate, .numOutputs = numOutputsForGate};
            strcpy(gates[numGates].type, directive);
            memcpy(gates[numGates].inputIndices, inputs, numInputsForGate * sizeof(int));
            memcpy(gates[numGates].outputIndices, outputs, numOutputsForGate * sizeof(int));
            numGates++;
        }
    }
    fclose(file);

    // Build dependency graph after reading all gates
    buildDependencyGraph();

    // Sort gates topologically
    resolveDependencies();

    // Evaluate and print the truth table
    printTruthTable();

    return EXIT_SUCCESS;
}