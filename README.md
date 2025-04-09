# Logic Circuit Truth Table Generator

This C program reads a text-based description of a digital circuit and outputs its complete truth table.

### 🔧 Features
- Supports logic gates: AND, OR, NOT, NAND, NOR, XOR, DECODER, MULTIPLEXER
- Reads a custom circuit description format
- Outputs all input/output combinations in truth table format
- Handles intermediate wires/variables and multi-gate circuits

### 📄 Sample Input
INPUT 3 a b c OUTPUT 1 d AND a b x AND c x d


### 🧪 Sample Output
0 0 0 | 0 0 0 1 | 0 ... 1 1 1 | 1


### 💡 Implementation Notes
- Parsed input with `fscanf()` and built an internal representation of the circuit
- Used arrays and enums to simulate logic gates
- Generated the table row-by-row to support large circuits

> Educational/research use only. Logic circuit descriptions are simplified for learning and demonstration purposes.
