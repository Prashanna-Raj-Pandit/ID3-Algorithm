###Program Information:
This C++ program reads data from ARFF files, stores them, and constructs a decision tree using the ID3 
algorithm. The program then prints the decision tree and provides a performance summary that includes 
the accuracy of the decision tree based on the input data.

The ID3 algorithm is a popular decision tree algorithm used in machine learning for classification tasks. 
It uses entropy and information gain to recursively split the data and build a tree that can be used 
to classify new instances.

This program supports three ARFF files:
1. contact-lenses.arff - Contains data about fitting contact lenses.
2. restaurant.arff - Contains data about whether a customer will wait for a table at a restaurant.
3. weather.nominal.arff - Contains weather data to decide whether to play outside.


###Features:
- Reads ARFF files and stores attribute and data information.
- Builds a decision tree using the ID3 algorithm.
- Prints the decision tree in a readable format with '|' symbols for branching.
- Provides a performance summary including the accuracy of the tree.

###Limitations:
- No separate test dataset is available, so the model's generalization ability cannot be evaluated.
- The model is likely overfitting the training data, as it has memorized the training examples.

