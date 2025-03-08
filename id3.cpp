/*
Author Name:
    -Prashanna Raj Pandit 
    -Asha Shah    
Southern Illinois University Edwardsville (SIUE)
--------------------------------------------------------------------------------------------------------
*/

#include <iostream> 
#include <fstream> 
#include <vector> 
#include <string> 
#include <sstream> 
#include <algorithm> 
#include <iomanip> 
#include <map> 
#include <cmath> 
using namespace std; 

string trim(const string &str) {
    size_t first = str.find_first_not_of(" \t\r\n"); // Find the first non-whitespace character
    if (first == string::npos) return ""; // If no non-whitespace character is found, return an empty string
    size_t last = str.find_last_not_of(" \t\r\n"); // Find the last non-whitespace character
    return str.substr(first, last - first + 1); // Return the trimmed substring
}

struct Attribute {
    string name; // Name of the attribute
    vector<string> values; // Possible values of the attribute
};

// Struct to represent a decision tree node
struct TreeNode {
    string attribute; // Attribute to split on (empty for leaf nodes)
    string decision; // Class label (for leaf nodes)
    map<string, TreeNode*> children; // Children nodes (key: attribute value)

    ~TreeNode() { // Destructor to clean up dynamically allocated memory
        for (auto& child : children) {
            delete child.second; // Delete each child node
        }
    }
};

// Function to split a string by a delimiter
vector<string> split(const string &s, char delimiter) {
    vector<string> tokens; // Vector to store the resulting tokens
    string token; // Temporary string to hold each token
    istringstream tokenStream(s); // Create a string stream from the input string
    while (getline(tokenStream, token, delimiter)) { // Split the string by the delimiter
        token = trim(token); // Trim whitespace from the token
        tokens.push_back(token); // Add the token to the vector
    }
    return tokens; // Return the vector of tokens
}

// Function to read ARFF file and correctly map data
bool readARFF(const string& filename, vector<Attribute>& attributes, vector<vector<string>>& data) {
    ifstream file(filename); // Open the file for reading
    if (!file.is_open()) { // Check if the file was successfully opened
        cerr << "Error opening file: " << filename << endl; // Print an error message
        return false; // Return false to indicate failure
    }

    string line; // String to hold each line of the file
    bool dataSection = false; // Flag to indicate if we are in the data section

    while (getline(file, line)) { // Read the file line by line
        line = trim(line); // Trim whitespace from the line
        if (line.empty() || line[0] == '%') continue; // Skip empty lines and comments

        if (line.find("@ATTRIBUTE") == 0 || line.find("@attribute") == 0) { // Check if the line defines an attribute
            istringstream iss(line); // Create a string stream from the line
            string token, attrName, values; // Temporary strings for parsing
            iss >> token; // Read the "@ATTRIBUTE" token

            size_t braceStart = line.find('{'); // Find the start of the attribute values
            if (braceStart != string::npos) { // If the attribute has values
                attrName = trim(line.substr(line.find(' ') + 1, braceStart - line.find(' ') - 1)); // Extract the attribute name
            } else {
                iss >> attrName; // Otherwise, read the attribute name directly
            }

            Attribute attr; // Create a new Attribute object
            attr.name = attrName; // Set the attribute name

            size_t braceEnd = line.find('}'); // Find the end of the attribute values
            if (braceStart != string::npos && braceEnd != string::npos) { // If the attribute has values
                string valuesStr = line.substr(braceStart + 1, braceEnd - braceStart - 1); // Extract the values
                attr.values = split(valuesStr, ','); // Split the values into a vector
            }
            attributes.push_back(attr); // Add the attribute to the vector
        } else if (line.find("@DATA") == 0 || line.find("@data") == 0) { // Check if the line marks the start of the data section
            dataSection = true; // Set the data section flag
        } else if (dataSection) { // If we are in the data section
            vector<string> row = split(line, ','); // Split the line into a row of values
            if (row.size() == attributes.size()) { // Check if the row has the correct number of values
                data.push_back(row); // Add the row to the data vector
            } else {
                cerr << "Warning: Data row has " << row.size() << " values, expected " << attributes.size() << "." << endl; // Print a warning if the row is invalid
            }
        }
    }

    file.close(); 
    return true; 
}

double calculateEntropy(const vector<vector<string>>& data, int classIndex) {
    map<string, int> classCounts; // Map to count occurrences of each class
    for (const auto& row : data) { // Iterate over each row in the data
        classCounts[row[classIndex]]++; // Increment the count for the class
    }

    double entropy = 0.0; // Initialize entropy to 0
    for (const auto& count : classCounts) { // Iterate over each class count
        double probability = static_cast<double>(count.second) / data.size(); // Calculate the probability of the class
        entropy -= probability * log2(probability); // Update the entropy
    }
    return entropy; 
}

// Function to calculate information gain
double calculateInformationGain(const vector<vector<string>>& data, int attributeIndex, int classIndex) {
    double entropyBefore = calculateEntropy(data, classIndex); // Calculate the entropy before splitting

    map<string, vector<vector<string>>> splits; // Map to store splits based on the attribute
    for (const auto& row : data) { // Iterate over each row in the data
        splits[row[attributeIndex]].push_back(row); // Add the row to the appropriate split
    }

    double entropyAfter = 0.0; // Initialize entropy after splitting to 0
    for (const auto& split : splits) { // Iterate over each split
        double probability = static_cast<double>(split.second.size()) / data.size(); // Calculate the probability of the split
        entropyAfter += probability * calculateEntropy(split.second, classIndex); // Update the entropy after splitting
    }

    return entropyBefore - entropyAfter; 
}

// Function to select the best attribute for splitting
int selectBestAttribute(const vector<vector<string>>& data, const vector<int>& attributeIndices, int classIndex) {
    int bestAttributeIndex = -1; // Initialize the best attribute index to -1
    double maxInformationGain = -1.0; // Initialize the maximum information gain to -1

    for (int attrIndex : attributeIndices) { 
        double informationGain = calculateInformationGain(data, attrIndex, classIndex); // Calculate the information gain
        if (informationGain > maxInformationGain) { // Check if this attribute has the highest information gain
            maxInformationGain = informationGain; // Update the maximum information gain
            bestAttributeIndex = attrIndex; // Update the best attribute index
        }
    }

    return bestAttributeIndex; 
}

// Function to build the ID3 decision tree
TreeNode* buildTree(const vector<vector<string>>& data, const vector<int>& attributeIndices, int classIndex) {
    TreeNode* node = new TreeNode(); // Create a new tree node

    // Check if all instances belong to the same class
    string firstClass = data[0][classIndex]; // Get the class of the first instance
    bool allSameClass = true; // Flag to check if all instances have the same class
    for (const auto& row : data) { // Iterate over each row in the data
        if (row[classIndex] != firstClass) { // Check if the class is different
            allSameClass = false; // Set the flag to false
            break; // Exit the loop
        }
    }
    if (allSameClass) { // If all instances have the same class
        node->decision = firstClass; // Set the node's decision to the class
        return node; // Return the node as a leaf node
    }

    // Check if no attributes are left to split on
    if (attributeIndices.empty()) { // If no attributes are left
        map<string, int> classCounts; // Map to count occurrences of each class
        for (const auto& row : data) { // Iterate over each row in the data
            classCounts[row[classIndex]]++; // Increment the count for the class
        }
        string majorityClass; // String to store the majority class
        int maxCount = 0; // Variable to store the maximum count
        for (const auto& count : classCounts) { // Iterate over each class count
            if (count.second > maxCount) { // Check if this class has the highest count
                maxCount = count.second; // Update the maximum count
                majorityClass = count.first; // Update the majority class
            }
        }
        node->decision = majorityClass; // Set the node's decision to the majority class
        return node; // Return the node as a leaf node
    }

    // Select the best attribute to split on
    int bestAttributeIndex = selectBestAttribute(data, attributeIndices, classIndex); // Get the best attribute index
    node->attribute = to_string(bestAttributeIndex); // Set the node's attribute to the best attribute

    // Split the data based on the best attribute
    map<string, vector<vector<string>>> splits; // Map to store splits based on the attribute
    for (const auto& row : data) { // Iterate over each row in the data
        splits[row[bestAttributeIndex]].push_back(row); // Add the row to the appropriate split
    }

    // Recursively build the tree for each split
    vector<int> remainingAttributes = attributeIndices; // Copy the remaining attributes
    remainingAttributes.erase(remove(remainingAttributes.begin(), remainingAttributes.end(), bestAttributeIndex), remainingAttributes.end()); // Remove the best attribute

    for (const auto& split : splits) { // Iterate over each split
        node->children[split.first] = buildTree(split.second, remainingAttributes, classIndex); // Recursively build the tree
    }

    return node; // Return the node
}

// Function to print the decision tree
void printTree(TreeNode* node, const vector<Attribute>& attributes, const string& prefix = "") {
    if (!node->attribute.empty()) { // If the node is not a leaf node
        // Print the current attribute and its children
        for (const auto& child : node->children) { // Iterate over each child
            cout << prefix << attributes[stoi(node->attribute)].name << " = " << child.first; // Print the attribute and value
            if (child.second->attribute.empty()) { // If the child is a leaf node
                // Print the class label
                cout << ": " << child.second->decision << endl;
            } else {
                // Recursively print the subtree
                cout << endl;
                printTree(child.second, attributes, prefix + "| ");
            }
        }
    }
}

// Function to print attributes
void printAttributes(const vector<Attribute>& attributes, const vector<vector<string>>& data) {
    cout << attributes.back().name << endl; // Print the name of the class attribute
    cout << "Attributes: " << attributes.size() << endl; // Print the number of attributes
    cout << "Examples: " << data.size() << endl << endl; // Print the number of data rows
}

// Function to evaluate the decision tree and calculate accuracy
double evaluateTree(TreeNode* node, const vector<vector<string>>& data, int classIndex, const vector<Attribute>& attributes) {
    int correct = 0; // Counter for correct predictions
    for (const auto& instance : data) { // Iterate over each instance in the data
        TreeNode* current = node; // Start at the root of the tree
        while (current->attribute != "") { // Traverse the tree until a leaf node is reached
            string attrValue = instance[stoi(current->attribute)]; // Get the attribute value
            if (current->children.find(attrValue) != current->children.end()) { // Check if the value exists in the children
                current = current->children[attrValue]; // Move to the child node
            } else {
                break; // If the value is not found, break the loop
            }
        }
        if (current->decision == instance[classIndex]) { // Check if the prediction is correct
            correct++; // Increment the correct counter
        }
    }
    return (double)correct / data.size(); // Return the accuracy
}

void printPerformanceSummary(TreeNode* root, const vector<vector<string>>& data, int classIndex, const vector<Attribute>& attributes) {
    double accuracy = evaluateTree(root, data, classIndex, attributes); // Calculate the accuracy
    cout << "Performance Summary:" << endl;
    cout << "Accuracy: " << fixed << setprecision(2) << accuracy * 100 << "%" << endl; // Print the accuracy
}

int main() {
    vector<string> filenames = { // List of ARFF files to process
        "contact-lenses.arff",
        "restaurant.arff",
        "weather.nominal.arff"
    };

    while (true) { // Main loop to process files
        cout << "Select the data file:" << endl;
        cout << "0. Exit" << endl;
        for (size_t i = 0; i < filenames.size(); ++i) { // Print the list of files
            cout << i + 1 << ". " << filenames[i] << endl;
        }
        cout << "Enter your choice (0-" << filenames.size() << "): ";
        int choice;
        cin >> choice; // Get the user's choice

        if (choice == 0) { // Exit the program if the user chooses 0
            cout << "Exiting program." << endl;
            break;
        }

        if (choice < 1 || choice > filenames.size()) { // Check if the choice is valid
            cout << "Invalid choice. Please try again." << endl;
            continue;
        }

        string filename = filenames[choice - 1]; // Get the selected filename
        vector<Attribute> attributes; // Vector to store attributes
        vector<vector<string>> data; // Vector to store data

        if (!readARFF(filename, attributes, data)) { // Read the ARFF file
            cerr << "Failed to read ARFF file." << endl;
            continue;
        }

        int classIndex = attributes.size() - 1; // Assume the last attribute is the class
        vector<int> attributeIndices; // Vector to store attribute indices
        for (int i = 0; i < attributes.size() - 1; ++i) { // Populate the attribute indices
            attributeIndices.push_back(i);
        }

        printAttributes(attributes, data); // Print the attributes and data summary
        TreeNode* root = buildTree(data, attributeIndices, classIndex); // Build the decision tree
        printTree(root, attributes); // Print the decision tree
        cout << endl;
        printPerformanceSummary(root, data, classIndex, attributes); // Print the performance summary
        cout << "\n\n";
        
        delete root;
    }

    return 0; 
}