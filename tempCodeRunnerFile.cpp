#include <iostream>
#include <string>
#include <ctime>//*         to calculate the time used as timestamp
#include <sstream>//*       to convert to stringstream
#include <functional>//*    for hash 
#include <fstream>//*       for filehandling 
#include <stack>
#include <queue>
#include <algorithm>//*     FOR SORTING NOT USED NOW USED BEFORE
#include <iomanip>

using namespace std;

//?    -- FORWARD DECLARATION BLOCKCHAIN CLASS AND BLOCK STRUCT --

class Blockchain;
struct block;  

struct Citizens{
    int id;
    string name;
    string address;
    string CNIC;
    int age;
    //!     DECLARED HERE DEFINED AFTER BLOCKCHAIN CLASS
    void inputCitizenData(Blockchain& blockchain);
    
};

struct Voting {
    string voterCNIC;       
    string candidateCNIC;   

  void inputVotingData(Blockchain& blockchain);
  
};
//TODO  FOR MAKING OFF CHAIN FAMILY TREE
struct FamilyNode {
    string CNIC;
    string name;
    FamilyNode* parent;
    vector<FamilyNode*> children;
    
    FamilyNode(string c, string n) : CNIC(c), name(n), parent(nullptr) {}
};


//!  CALCULATIN HASH OF THE DATA SEPERATE FOR CITIZEN AND VOTING USED SSTREAM FOR STRING STREAM AND FUNCTIONAL FOR HASH

string calculateHash(int index, Citizens citizenData, Voting voteData, time_t timestamp, string prevhash, int BlockType) {
    stringstream ss;
    ss << index << timestamp << prevhash << BlockType;

    if (BlockType == 0) { //* REGISTRATION
        ss << citizenData.id << citizenData.name << citizenData.CNIC << citizenData.address << citizenData.age;
    } else if (BlockType == 1) { //* VOTING
        ss << voteData.voterCNIC << voteData.candidateCNIC;
    }

    string combinedData = ss.str();
    hash<string> hasher;
    size_t hashValue = hasher(combinedData);
    return to_string(hashValue);
}

//TODO      BLOCK WHICH IS SHOWING THE DATA INSIDE THE BLOCK & ENUM FOR BLOCK TYPE 0 FOR REGISTRATION AND 1 FOR VOTING
struct block{
    
    enum BlockType{
        REGISTRATION,
        VOTING 
    };

    int Index;
    Citizens data;
    Voting voteData;
    time_t timestamp;
    string hash;
    string prevhash;
    BlockType type;
    block* next;

    //? BY DEFAULT IT WILL BE NULL
    
    block(){
        Index = 0;
        data.id = 0;
        data.name = "";
        data.address = "";
        data.CNIC = "";
        data.age = 0;
        voteData.candidateCNIC = "";
        voteData.voterCNIC = "";
        timestamp = time(0);
        hash = "";
        prevhash = "";
        type = REGISTRATION;
        next = NULL;
    }
    //? CONSTRUCTOR FOR REGISTRATION USER WILL PASS INDEX CITIZEN DATA & PREV HASH
    block(int idx, Citizens d, string prevHashVal) {
        Index = idx;
        data = d;
        timestamp = time(0);
        type = REGISTRATION;
        voteData = {};  // empty vote
        prevhash = prevHashVal;
        hash = calculateHash(Index, data, voteData, timestamp, prevhash, type);
        next = nullptr;
    }

    //? CONSTRUCTOR FOR VOTING USER WILL PASS INDEX VOTE DATA & PREV HASH
    block(int idx, Voting v, string prevHashVal) {
        Index = idx;
        voteData = v;
        timestamp = time(0);
        type = VOTING;
        data = {};  // empty citizen data
        prevhash = prevHashVal;
        hash = calculateHash(Index, data, voteData, timestamp, prevhash, type);
        next = nullptr;
    }
};

//? BLOCKCHAIN CLASS WHICH WILL HANDLE ALL BLOCKS IN LINKEDLIST 

 class Blockchain{
    public:
    block* head;
    int currentIndex = 0;

    vector<FamilyNode*> familyNodes; //* Stores all family nodes
    
    Blockchain(){
        head = nullptr;
    }

   void addRegistrationBlock(Citizens citizenData) {
    
    string prevHash;
    if (head == nullptr) {
        prevHash = "";
    }   else {
        prevHash = head->hash;
    }
        block* newBlock = new block(currentIndex++,citizenData, prevHash);
        newBlock->next = head;
        head = newBlock;
    }

    
   void addVotingBlock(Voting voterData) {
        
    string prevHash;
    if (head == nullptr) {
        prevHash = "";
    }   else {
        prevHash = head->hash;
    }
        block* newBlock = new block(currentIndex++,voterData, prevHash);
        newBlock->next = head;
        head = newBlock;
    }
    //! FOR FAMILY NODES
    block* findCitizenBlock(string cnic) {
        block* current = head;
        while (current) {
            if (current->type == block::REGISTRATION && 
                current->data.CNIC == cnic) {
                return current;
            }
            current = current->next;
        }
        return nullptr;
    }
    //* FOR FAMILY NODES (family relationship function)

    bool addFamilyRelationship(string childCNIC, string parentCNIC) {
        // 1. Verify both CNICs exist
        if (!isRegisteredVoter(childCNIC)) {
            cout << "Child CNIC not registered!\n";
            return false;
        }
        if (!parentCNIC.empty() && !isRegisteredVoter(parentCNIC)) {
            cout << "Parent CNIC not registered!\n";
            return false;
        }

        // 2. Find or create child node
        FamilyNode* child = nullptr;
        for (FamilyNode* node : familyNodes) {
            if (node->CNIC == childCNIC) {
                child = node;
                break;
            }
        }
        if (!child) {
            block* b = findCitizenBlock(childCNIC);
            child = new FamilyNode(childCNIC, b->data.name);
            familyNodes.push_back(child);
        }

        // 3. If no parent specified, we're done
        if (parentCNIC.empty()) {
            return true;
        }

        // 4. Find or create parent node
        FamilyNode* parent = nullptr;
        for (FamilyNode* node : familyNodes) {
            if (node->CNIC == parentCNIC) {
                parent = node;
                break;
            }
        }
        if (!parent) {
            block* b = findCitizenBlock(parentCNIC);
            parent = new FamilyNode(parentCNIC, b->data.name);
            familyNodes.push_back(parent);
        }

        // 5. Set relationship
        child->parent = parent;
        parent->children.push_back(child);
        
        return true;
    }

    //! Function to display family tree (all in one public function)

  void displayFamily(string cnic) {
    // 1. Find the person
    FamilyNode* person = nullptr;
    for (FamilyNode* node : familyNodes) {
        if (node->CNIC == cnic) {
            person = node;
            break;
        }
    }

    if (!person) {
        cout << "Person not found in family records!\n";
        return;
    }

    // 2. Find the root of the family
    FamilyNode* root = person;
    while (root->parent != nullptr) {
        root = root->parent;
    }

    // 3. Simple display using generation levels
    cout << "\nFamily Tree for " << root->name << " (" << root->CNIC << "):\n";
    
    vector<FamilyNode*> currentGeneration = {root};
    int level = 0;
    
    while (!currentGeneration.empty()) {
        vector<FamilyNode*> nextGeneration;
        
        // Print all in current generation
        for (FamilyNode* member : currentGeneration) {
            // Indent based on level
            for (int i = 0; i < level; i++) cout << "  ";
            
            // Different symbols for different levels
            if (level == 0) {
                cout << "Root: ";
            } else {
                cout << "-> ";
            }
            
            cout << member->name << " (" << member->CNIC << ")\n";
            
            // Add children to next generation
            for (FamilyNode* child : member->children) {
                nextGeneration.push_back(child);
            }
        }
        
        currentGeneration = nextGeneration;
        level++;
    }
}

//! TO SAVE DATA IN FILE
void saveFamilyTreeToFile(const string& filename) {
    ofstream file(filename);
    
    if (!file) {
        cout << "Error opening family tree file!\n";
        return;
    }

    // Find all root nodes (people without parents)
    vector<FamilyNode*> roots;
    for (FamilyNode* node : familyNodes) {
        if (node->parent == nullptr) {
            roots.push_back(node);
        }
    }

    // Save each family tree
    for (FamilyNode* root : roots) {
        file << "Family of " << root->name << " (" << root->CNIC << "):\n";
        
        vector<pair<FamilyNode*, int>> toProcess = {{root, 0}}; // node and level
        
        while (!toProcess.empty()) {
            auto current = toProcess.back();
            toProcess.pop_back();
            
            // Indent based on level
            for (int i = 0; i < current.second; i++) file << "  ";
            file << "-> " << current.first->name << " (" << current.first->CNIC << ")\n";
            
            // Add children to process (in reverse order)
            for (int i = current.first->children.size()-1; i >= 0; i--) {
                toProcess.push_back({current.first->children[i], current.second + 1});
            }
        }
        
        file << "\n"; // Separate families
    }
    
    file.close();
    cout << "Family tree saved to " << filename << "\n";
}
    //! THE FUCNCTION TO VERIFY THE CNIC USED IN INPUT CITIZEN 

    bool isCNICUnique(const string& cnic) {
        block* current = head;
        while (current != nullptr) {
            if (current->type == block::REGISTRATION && 
                current->data.CNIC == cnic) {
                return false; // CNIC already exists
            }
            current = current->next;
        }
        return true; // CNIC is unique
    }

    //? FOR CHECKING THE CNIC IN BLOCKCHAIN ONLY THAT PERSON CAN VOTE

    bool isRegisteredVoter(const string& cnic) {
        block* current = head;
        while (current != nullptr) {
            if (current->type == block::REGISTRATION && 
            current->data.CNIC == cnic) {
            return true; // CNIC is registered (can vote)
            }
            current = current->next;
        }
        return false; // CNIC not found 
    }
    //! CHECK IF VOTER HAVE VOTED 
    bool hasAlreadyVoted(const string& cnic) {
        block* current = head;
        while (current != nullptr) {
            if (current->type == block::VOTING && 
                current->voteData.voterCNIC == cnic) {
                return true; // Vote found for this CNIC
            }
            current = current->next;
        }
        return false; // No vote found
    }
    //! TO VERIFY AND COUNT 
    void verifyVote(const string& voterCNIC) {
        block* current = head;
        while (current != nullptr) {
            if (current->type == block::VOTING && 
                current->voteData.voterCNIC == voterCNIC) {
                cout << "\n=== VOTE VERIFICATION ===\n";
                cout << "Voter CNIC: " << voterCNIC << "\n";
                cout << "Voted for: " << current->voteData.candidateCNIC << "\n";
                cout << "Block Index: " << current->Index << "\n";
                cout << "Timestamp: " << ctime(&current->timestamp);
                return;
            }
            current = current->next;
        }
        cout << "No vote found for CNIC: " << voterCNIC << "\n";
    }
    //! FUNCTION TO DISPLAY THE BLOCKCHAIN 

    void displayBlockchain() {
        block* current = head;
        while (current != nullptr) {
            cout <<"------------------------"<<endl;
            cout << "Block Index: " << current->Index << endl;
            cout << "Timestamp: " << ctime(&(current->timestamp)); // nicer time display
            cout << "Hash: " << current->hash << endl;
            cout << "Previous Hash: " << current->prevhash << endl;
            cout << "BlockType: " << endl;
            if (current->type == block::REGISTRATION) {
                cout<<"**Registration**"<<endl;
                cout << "Citizen ID: " << current->data.id << endl;
                cout << "Citizen Name: " << current->data.name << endl;
                cout << "Citizen Address: " << current->data.address << endl;
                cout << "Citizen CNIC: " << current->data.CNIC << endl;
                cout << "Citizen Age: " << current->data.age << endl;
            } else if (current->type == block::VOTING) {
                cout<<"**Voting**"<<endl;
                cout << "Voter CNIC: " << current->voteData.voterCNIC << endl;
                cout << "Candidate CNIC: " << current->voteData.candidateCNIC << endl;
            }
            cout << "-----------------------------------" << endl;
            current = current->next;
        }
    }
//! HERE WE ARE MAINTANING THE BLOCKCHAIN AND SAVING THE DATA IN A FILE WITHOUT ANY KIND OF MODIFICATION
//TODO BASICALY WE ARE USING ON CHAIN DATA OFF CHAIN MAINTAINING ITS INTAGRITY
    void saveToFile(const string& filename) {
        ofstream file(filename);

        if (!file) {
        cout << "Error opening file!" << endl;
            return;
        }

    block* current = head;
    while (current != nullptr) {
        file << "===============================" << endl;
        file << "        BLOCK DETAILS          " << endl;
        file << "===============================" << endl;

        file << "Block Index     : " << current->Index << endl;
        file << "Timestamp       : " << ctime(&(current->timestamp));
        file << "Block Type      : ";

        if (current->type == block::REGISTRATION) {
            file << "REGISTRATION" << endl;
            file << "Citizen ID      : " << current->data.id << endl;
            file << "Name            : " << current->data.name << endl;
            file << "Address         : " << current->data.address << endl;
            file << "CNIC            : " << current->data.CNIC << endl;
            file << "Age             : " << current->data.age << endl;
        } else if (current->type == block::VOTING) {
            file << "VOTING" << endl;
            file << "Voter CNIC      : " << current->voteData.voterCNIC << endl;
            file << "Candidate CNIC  : " << current->voteData.candidateCNIC << endl;
        }

        file << "Hash            : " << current->hash << endl;
        file << "Previous Hash   : " << current->prevhash << endl;

        file << "\n-----------------------------------\n\n";

        current = current->next;
    }

    file.close();
    cout << "Blockchain saved to file: " << filename << endl;
}

    //! THIS IS AN ADDITIONAL FUNCTION JUST TO SHOW THE BLOCKCHAIN INTAGRITY AND SECURITY
    //? THE MINOR CHANGES IN THE BLOCKCHAIN WILL BE DETECTED BY THIS FUNCTION BECAUSE THE HASH WILL CHANGE

    bool verifyBlockchain() {
        block* current = head;

        while (current != nullptr && current->next != nullptr) {
        
        //! Recalculate hash of current block
        string recalculatedHash = calculateHash(
            current->Index,
            current->data,
            current->voteData,
            current->timestamp,
            current->prevhash,
            current->type
        );

        //! Check if stored hash matches recalculated hash
        if (current->hash != recalculatedHash) {
            cout << "Hash mismatch at Block Index: " << current->Index << endl;
            return false;
        }

        //! Check if current block's prevhash matches next block's hash
        if (current->prevhash != current->next->hash) {
            cout << "Invalid link between Block " << current->Index
                 << " and Block " << current->next->Index << endl;
            return false;
        }

        current = current->next;
    }

    cout << "Blockchain is valid!" << endl;
    return true;
}


};

//? FUNCTION IS BELOW BECAUSE WE NEEDED TO USE BLOCKCHAIN CLASS OBJECT 

void Citizens::inputCitizenData(Blockchain& blockchain) {
    cout << "Enter ID: ";
    cin >> id;
    cin.ignore(); 
    cout << "Enter Name: ";
    getline(cin, name);
    cout << "Enter Address: ";
    getline(cin, address);
    
    //!   -- CHECKING CNIC THAT IF IT IS UNIQUE OR NOT AND MUST BE 5 CHARS
    while(true) {
        cout << "Enter CNIC (5 characters): ";
        getline(cin, CNIC);
        
        if(CNIC.length() != 5) {
            cout << "Error: CNIC must be exactly 5 characters\n";
            continue;
        }
        
        if(!blockchain.isCNICUnique(CNIC)) {
            cout << "Error: This CNIC is already registered\n";
        } else {
            break;
        }
    }
    
    cout << "Enter Age: ";
    cin >> age;
}
//! CHECKING THAT IF VOTER CNIC IS REGISTERD IN BLOCKCHAIN 

void Voting::inputVotingData(Blockchain& blockchain) {
        //TODO Voter validation
        while(true) {
            cout << "Enter Voter CNIC: ";
            cin >> voterCNIC;
            
            if(!blockchain.isRegisteredVoter(voterCNIC)) {
                cout << "Error: Not a registered voter!\n";
            } else if(blockchain.hasAlreadyVoted(voterCNIC)) {
                cout << "Error: This voter has already cast a vote!\n"; 
            } else {
                break;
            }
        }

        //TODO Candidate validation
        while(true) {
            cout << "Enter Candidate CNIC: ";
            cin >> candidateCNIC;
            
            if(candidateCNIC == voterCNIC) {
                cout << "Error: Cannot vote for yourself!\n";
            } else {
                break;
            }
        }
    }

//! THIS PART IS FOR OFF CHAIN WE ARE CREATING A BLOCKDATA STRUCT SO WE DONT TEMPER THE ACTUAL DATA 

const int MAX_BLOCKS = 100;  //! Fixed size array for storage

struct BlockData {
    int Index;
    string CNIC;
    string Name; //? Used only for registration blocks
    int type; //? 0: Registration, 1: Voting
};

//! Utility to extract data from blockchain to array

int exportToArray(Blockchain& chain, BlockData arr[]) {
    block* current = chain.head;
    int count = 0;
    while (current != nullptr && count < MAX_BLOCKS) {
        arr[count].Index = current->Index;
        arr[count].type = current->type;

        if (current->type == block::REGISTRATION) {
            arr[count].CNIC = current->data.CNIC;
            arr[count].Name = current->data.name;
        } else if (current->type == block::VOTING) {
            arr[count].CNIC = current->voteData.voterCNIC;
            arr[count].Name = "N/A";
        }

        current = current->next;
        count++;
    }
    return count;
}

//! BUBBLE SORTING THE OFF CHAIN DATA ON THE BASES OF CNIC
void sortByCNIC(BlockData arr[], int size) {
   
    //? Bubble sort implementation
   
    for (int i = 0; i < size-1; i++) {
        bool swapped = false; //TODO Optimization flag
        
        for (int j = 0; j < size-i-1; j++) {
            //TODO Compare adjacent CNICs
            if (arr[j].CNIC > arr[j+1].CNIC) {
                //TODO Swap the elements
                BlockData temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
                swapped = true;
            }
        }
        
        //TODO If no swaps in inner loop, array is sorted
        if (!swapped) break;
    }

    //TODO Display the sorted data (unchanged from your original)
    cout << "\n--- Sorted Data by CNIC (Using Bubble Sort) ---\n";
    for (int i = 0; i < size; i++) {
        cout << "Index: " << arr[i].Index 
             << " | CNIC: " << arr[i].CNIC 
             << " | Name: " << arr[i].Name 
             << " | Type: " << (arr[i].type == 0 ? "Registration" : "Voting") 
             << endl;
    }
}

//! IMPLEMENTING THE INSERTION SORTING
void insertionSortByCNIC(BlockData arr[], int size) {
    for (int i = 1; i < size; i++) {
        BlockData key = arr[i];
        int j = i - 1;
        
        while (j >= 0 && arr[j].CNIC > key.CNIC) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
    //TODO Display the sorted data (unchanged from your original)
    cout << "\n--- Sorted Data by CNIC (Using INSERTION Sort) ---\n";
        
        for (int i = 0; i < size; i++) {
            cout << "Index: " << arr[i].Index 
             << " | CNIC: " << arr[i].CNIC 
             << " | Name: " << arr[i].Name 
             << " | Type: " << (arr[i].type == 0 ? "Registration" : "Voting") 
             << endl;
        }
}

//! IMPLEMENTING THE SELECTION SORTING
void selectionSortByCNIC(BlockData arr[], int size) {
    for (int i = 0; i < size-1; i++) {
        int min_idx = i;
        for (int j = i+1; j < size; j++) {
            if (arr[j].CNIC < arr[min_idx].CNIC)
                min_idx = j;
        }
        swap(arr[min_idx], arr[i]);
    }
     //TODO Display the sorted data (unchanged from your original)
    cout << "\n--- Sorted Data by CNIC (Using SELECTION Sort) ---\n";
        
        for (int i = 0; i < size; i++) {
            cout << "Index: " << arr[i].Index 
             << " | CNIC: " << arr[i].CNIC 
             << " | Name: " << arr[i].Name 
             << " | Type: " << (arr[i].type == 0 ? "Registration" : "Voting") 
             << endl;
        }
}

//! STACK VIEW 
void displayStack(BlockData arr[], int size) {
    stack<BlockData> s;
    for (int i = 0; i < size; i++)
        s.push(arr[i]);

    cout << "\n--- Stack View (LIFO) ---\n";
    while (!s.empty()) {
        BlockData top = s.top();
        s.pop();
        cout << "Index: " << top.Index 
             << " | CNIC: " << top.CNIC 
             << " | Name: " << top.Name 
             << " | Type: " << (top.type == 0 ? "Registration" : "Voting") 
             << endl;
    }
}

//* QUEUE VIEW
void displayQueue(BlockData arr[], int size) {
    queue<BlockData> q;
    for (int i = 0; i < size; i++)
        q.push(arr[i]);

    cout << "\n--- Queue View (FIFO) ---\n";
    while (!q.empty()) {
        BlockData front = q.front();
        q.pop();
        cout << "Index: " << front.Index 
             << " | CNIC: " << front.CNIC 
             << " | Name: " << front.Name 
             << " | Type: " << (front.type == 0 ? "Registration" : "Voting") 
             << endl;
    }
}

int main() {
    Blockchain blockchain;
    BlockData blockArray[MAX_BLOCKS];
    int count = 0;

    int choice;
    do {
        cout << "**********************************"<<endl;
        cout <<setw(5)<< "-- WELCOME TO M.STATE E-SYSTEM --"<<endl;
        cout << "**********************************"<<endl;
        cout << "1. Add new citizen (Registration)\n";
        cout << "2. Add new vote (Voting)\n";
        cout << "3. Display blockchain\n";
        cout << "4. Save blockchain to file\n";
        cout << "5. Export and sort data\n";
        cout << "6. Display using Stack\n";
        cout << "7. Display using Queue\n";
        cout << "8. Verify Blockchain Integrity\n";
        cout << "9. Verify Individual Vote\n"; 
        cout << "10. Add family relationship\n";
        cout << "11. Display family tree\n";
        cout << "12. Save family tree to file\n";
        cout << "0. Exit\n";
        cout <<"---"<<endl;
        cout << "Enter choice: ";
        cin >> choice;
        cin.ignore();

        switch (choice) {
            case 1: {
                Citizens c;
                c.inputCitizenData(blockchain);
                blockchain.addRegistrationBlock(c);
                break;
            }
            case 2: {
                Voting v;
                v.inputVotingData(blockchain);
                blockchain.addVotingBlock(v);
                break;
            }
            case 3:
                blockchain.displayBlockchain();
                break;
            case 4:
                blockchain.saveToFile("blockchain_data.txt");
                break;
            case 5:
                cout << "    a. Bubble Sort\n";
                cout << "    b. Insertion Sort\n";
                cout << "    c. Selection Sort\n";
                
                count = exportToArray(blockchain, blockArray);
                char sortChoice;
                cout << "Choose sorting algorithm (a/b/c): ";
                cin >> sortChoice;
    
                switch(tolower(sortChoice)) {

                case 'a':
                        sortByCNIC(blockArray, count); //*  bubble sort
                    break;
                case 'b':
                        insertionSortByCNIC(blockArray, count);//* insertion sort
                    break;
                case 'c':
                        selectionSortByCNIC(blockArray, count);//* selection sort
                    break;
                default:
                    cout << "Invalid choice, using Bubble Sort\n";
                    sortByCNIC(blockArray, count);
                }
                break;
            case 6:
                displayStack(blockArray, count);
                break;
            case 7:
                displayQueue(blockArray, count);
                break;
            case 8:
                blockchain.verifyBlockchain();
                break;
            case 9: {
                string cnic;
                cout << "Enter voter CNIC to verify: ";
                cin >> cnic;
                blockchain.verifyVote(cnic);
                
                break;
            }
            case 10: {
                string child, parent;
                cout << "Enter child CNIC: ";
                cin >> child;
                cout << "Enter parent CNIC (leave empty if none): ";
                cin.ignore();
                getline(cin, parent);
                if (blockchain.addFamilyRelationship(child, parent)) {
                    cout << "Family relationship added!\n";
                }
                break;
            }
            case 11: {
                string cnic;
                cout << "Enter CNIC to view family: ";
                cin >> cnic;
                blockchain.displayFamily(cnic);
                break;
            }
            case 12: {
                blockchain.saveFamilyTreeToFile("family_tree.txt");
                break;
            }
            case 0:
                cout << "Exiting program.\n";
                break;
            default:
                cout << "Invalid option. Try again.\n";
        }

    } while (choice != 0);

    return 0;
}


