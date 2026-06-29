//============================================================================
// ProjectTwo.cpp
// Course:   CS 300
// Author:   Dominick Griggs
//
// Description:
//   ABCU Advising Assistance Program. Loads course data from a CSV file
//   into a Binary Search Tree (BST). Provides a menu that allows academic
//   advisors to print an alphanumeric course list or look up a specific
//   course's title and prerequisites.
//
// Data Structure Rationale:
//   A BST was chosen because its in-order traversal produces alphanumeric
//   output with no additional sort step, and average-case search is O(log n),
//   making both required operations efficient.
//
//============================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

//============================================================================
// Data Structures
//============================================================================

// Holds all data for a single course
struct Course {
    string courseNumber;          // e.g. "CSCI300"
    string courseTitle;           // e.g. "Introduction to Algorithms"
    vector<string> prerequisites; // prerequisite course numbers
};

// BST node wrapping a Course
struct Node {
    Course course;
    Node*  left;
    Node*  right;

    explicit Node(const Course& c)
        : course(c), left(nullptr), right(nullptr) {}
};

//============================================================================
// BinarySearchTree
//============================================================================

class BinarySearchTree {
public:
    BinarySearchTree();
    ~BinarySearchTree();

    void   insert(const Course& course);
    void   printInOrder() const;          // Option 2 – sorted course list
    Course* search(const string& number); // Option 3 – course lookup
    bool   isEmpty() const;

private:
    Node* root;

    // Recursive helpers
    Node* insertRecursive(Node* node, const Course& course);
    void  inOrderRecursive(Node* node) const;
    Node* searchRecursive(Node* node, const string& number) const;
    void  destroyRecursive(Node* node);
};

BinarySearchTree::BinarySearchTree() : root(nullptr) {}

BinarySearchTree::~BinarySearchTree() {
    destroyRecursive(root);
}

bool BinarySearchTree::isEmpty() const {
    return root == nullptr;
}

// Public insert
void BinarySearchTree::insert(const Course& course) {
    root = insertRecursive(root, course);
}

// Recursively find the correct leaf position and insert
Node* BinarySearchTree::insertRecursive(Node* node, const Course& course) {
    if (node == nullptr) {
        return new Node(course);
    }
    if (course.courseNumber < node->course.courseNumber) {
        node->left  = insertRecursive(node->left,  course);
    } else if (course.courseNumber > node->course.courseNumber) {
        node->right = insertRecursive(node->right, course);
    }
    // Duplicate course numbers are silently ignored
    return node;
}

// Public print – calls recursive in-order traversal
void BinarySearchTree::printInOrder() const {
    inOrderRecursive(root);
}

// In-order traversal: left → visit → right produces alphanumeric order
void BinarySearchTree::inOrderRecursive(Node* node) const {
    if (node == nullptr) return;
    inOrderRecursive(node->left);
    cout << node->course.courseNumber << ", "
         << node->course.courseTitle  << endl;
    inOrderRecursive(node->right);
}

// Public search – returns pointer to Course or nullptr if not found
Course* BinarySearchTree::search(const string& number) {
    Node* result = searchRecursive(root, number);
    return (result != nullptr) ? &result->course : nullptr;
}

// Recursive BST search using alphanumeric ordering
Node* BinarySearchTree::searchRecursive(Node* node, const string& number) const {
    if (node == nullptr)                   return nullptr;
    if (number == node->course.courseNumber) return node;
    if (number <  node->course.courseNumber)
        return searchRecursive(node->left,  number);
    return searchRecursive(node->right, number);
}

// Post-order deletion to free all heap memory
void BinarySearchTree::destroyRecursive(Node* node) {
    if (node == nullptr) return;
    destroyRecursive(node->left);
    destroyRecursive(node->right);
    delete node;
}

//============================================================================
// Utility Functions
//============================================================================

// Convert a string to uppercase (enables case-insensitive course lookup)
string toUpperCase(string str) {
    transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

// Trim leading and trailing whitespace from a string
string trim(const string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

//============================================================================
// loadCourses
//
// Reads the specified CSV file and populates the BST.
//
// Two-pass strategy:
//   Pass 1 – collect every valid course number into allNumbers so that
//             prerequisite references can be validated.
//   Pass 2 – build Course objects, validate prerequisites, insert into BST.
//
// Handles:
//   - Missing / unreadable files
//   - Lines with fewer than 2 fields (invalid records)
//   - Trailing commas that produce empty tokens
//   - Windows-style \r\n line endings
//   - Prerequisites that reference a course not in the file
//============================================================================

void loadCourses(const string& fileName, BinarySearchTree& bst) {

    ifstream file(fileName);

    // Verify the file opened successfully
    if (!file.is_open()) {
        cout << "Error: Could not open file \"" << fileName << "\"." << endl;
        cout << "Please verify the file name and path, then try again." << endl;
        return;
    }

    // ── Pass 1: collect all course numbers ──────────────────────────────
    vector<string> allNumbers;
    string line;

    while (getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;

        stringstream ss(line);
        string token;
        vector<string> tokens;

        while (getline(ss, token, ',')) {
            token = trim(token);
            if (!token.empty()) tokens.push_back(token);
        }

        // A valid record must have at least courseNumber and courseTitle
        if (tokens.size() < 2) continue;

        allNumbers.push_back(toUpperCase(tokens[0]));
    }

    if (allNumbers.empty()) {
        cout << "Error: No valid course records found in \""
             << fileName << "\"." << endl;
        file.close();
        return;
    }

    // ── Pass 2: build and insert Course objects ──────────────────────────
    file.clear();
    file.seekg(0, ios::beg); // rewind to start of file

    int loadedCount  = 0;
    int skippedCount = 0;

    while (getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;

        stringstream ss(line);
        string token;
        vector<string> tokens;

        while (getline(ss, token, ',')) {
            token = trim(token);
            if (!token.empty()) tokens.push_back(token);
        }

        // Skip lines that lack the minimum required fields
        if (tokens.size() < 2) {
            cout << "Warning: Skipping invalid record: \"" << line << "\"" << endl;
            ++skippedCount;
            continue;
        }

        Course course;
        course.courseNumber = toUpperCase(tokens[0]);
        course.courseTitle  = tokens[1];

        // Validate and store each prerequisite (tokens index 2 and beyond)
        for (size_t i = 2; i < tokens.size(); ++i) {
            string prereq = toUpperCase(tokens[i]);

            // Check that the prerequisite exists in the course catalog
            bool found = false;
            for (const string& num : allNumbers) {
                if (num == prereq) { found = true; break; }
            }

            if (found) {
                course.prerequisites.push_back(prereq);
            } else {
                cout << "Warning: Prerequisite \"" << prereq
                     << "\" listed for \"" << course.courseNumber
                     << "\" does not exist in the course catalog. "
                     << "Skipping that prerequisite." << endl;
            }
        }

        bst.insert(course);
        ++loadedCount;
    }

    file.close();

    // Summary feedback to the user
    cout << loadedCount << " course(s) loaded successfully.";
    if (skippedCount > 0) {
        cout << " (" << skippedCount << " invalid record(s) skipped.)";
    }
    cout << endl;
}

//============================================================================
// printCourseInfo
//
// Looks up a course by number (case-insensitive) and displays its title and
// prerequisites. Displays an error message if the course is not found.
//============================================================================

void printCourseInfo(BinarySearchTree& bst, const string& input) {
    string courseNumber = toUpperCase(input);
    Course* course = bst.search(courseNumber);

    if (course == nullptr) {
        cout << "Error: Course \"" << courseNumber
             << "\" was not found in the data structure." << endl;
        return;
    }

    // Print course number and title
    cout << course->courseNumber << ", " << course->courseTitle << endl;

    // Print prerequisites
    if (course->prerequisites.empty()) {
        cout << "Prerequisites: None" << endl;
    } else {
        cout << "Prerequisites: ";
        for (size_t i = 0; i < course->prerequisites.size(); ++i) {
            if (i > 0) cout << ", ";
            cout << course->prerequisites[i];
        }
        cout << endl;
    }
}

//============================================================================
// displayMenu
//
// Prints the main menu to standard output.
//============================================================================

void displayMenu() {
    cout << "\n";
    cout << "  1. Load Data Structure.\n";
    cout << "  2. Print Course List.\n";
    cout << "  3. Print Course.\n";
    cout << "  9. Exit\n";
    cout << "\n";
    cout << "What would you like to do? ";
}

//============================================================================
// main
//============================================================================

int main() {

    BinarySearchTree bst;    // BST that stores all loaded Course objects
    bool dataLoaded = false; // Guard: prevents options 2 & 3 before load
    int  choice     = 0;

    cout << "Welcome to the course planner." << endl;

    // ── Main menu loop ───────────────────────────────────────────────────
    while (choice != 9) {

        displayMenu();

        // Read menu selection; recover gracefully from non-integer input
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Invalid input. Please enter a menu number." << endl;
            continue;
        }
        cin.ignore(1000, '\n'); // discard remainder of input line

        switch (choice) {

            // ── Option 1: Load data from CSV file ────────────────────────
            case 1: {
                string fileName;
                cout << "Enter the file name: ";
                getline(cin, fileName);
                fileName = trim(fileName);
                loadCourses(fileName, bst);
                dataLoaded = !bst.isEmpty();
                break;
            }

            // ── Option 2: Print alphanumeric course list ──────────────────
            case 2: {
                if (!dataLoaded) {
                    cout << "Error: No data loaded. "
                         << "Please use Option 1 to load the data structure first."
                         << endl;
                    break;
                }
                cout << "\nHere is a sample schedule:\n\n";
                bst.printInOrder();
                break;
            }

            // ── Option 3: Print individual course information ─────────────
            case 3: {
                if (!dataLoaded) {
                    cout << "Error: No data loaded. "
                         << "Please use Option 1 to load the data structure first."
                         << endl;
                    break;
                }
                string courseNumber;
                cout << "What course do you want to know about? ";
                getline(cin, courseNumber);
                courseNumber = trim(courseNumber);
                printCourseInfo(bst, courseNumber);
                break;
            }

            // ── Option 9: Exit ────────────────────────────────────────────
            case 9:
                cout << "Thank you for using the course planner!" << endl;
                break;

            // ── Any other value: invalid option ───────────────────────────
            default:
                cout << choice << " is not a valid option." << endl;
                break;
        }
    }

    return 0;
}