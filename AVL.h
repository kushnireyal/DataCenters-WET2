#ifndef DATACENTERS_WET1_AVL_H
#define DATACENTERS_WET1_AVL_H

#include <iostream>
#include "ServerRankTree.h"

using namespace std;
#define COUNT 10 // used in tree print function

enum AVLResult { AVL_SUCCESS, AVL_FAILURE, AVL_INVALID_INPUT, AVL_ALREADY_EXIST, AVL_NOT_EXIST };

template <class KeyType, class DataType>
class DefTreeNode {
public:
    KeyType key;
    DataType data;
    DefTreeNode* parent, * left, * right;
    int height;

    DefTreeNode(KeyType key,
             DataType data,
             DefTreeNode* parent = nullptr) :
            key(key), data(data),
            parent(parent), left(nullptr), right(nullptr),
            height(0) {};

    int getBalanceFactor() const;
    bool isLeftSubtree() const;
    bool isLeaf() const;
    bool hasSingleSon() const;
    bool hasTwoSons() const;
    virtual void updateRanks();

    static void swap(DefTreeNode& A, DefTreeNode& B);
};

template <class KeyType, class DataType, class TreeNode = DefTreeNode<KeyType, DataType>*>
class AVL {
public:
    class TreeIterator {
    public:
        TreeIterator() : curr(nullptr), last(nullptr) {};
        DataType& operator*() const;
        const TreeIterator operator++(int);
        const TreeIterator operator--(int);
        bool operator<(const TreeIterator& other) const;
        bool operator==(const TreeIterator& other) const;
        bool operator!=(const TreeIterator& other) const;

        friend AVL;
    protected:
        TreeNode *curr, * last;
    };

    AVL();
    ~AVL();
    TreeIterator find(const KeyType& key) const;
    AVLResult insert(const KeyType& key, const DataType& data);
    AVLResult remove(const KeyType& key);
    TreeIterator begin() const;
    TreeIterator end() const;
    TreeIterator Rbegin() const;
    int getSize() const;

    void printTree();

protected:
    // the actual tree is the dummy's left subtree
    TreeNode* dummyRoot;
    int size;

    virtual void fixTree(TreeNode* root);
    void BalanceSubTree(TreeNode* root);
    virtual void rotateRight(TreeNode* root);
    virtual void rotateLeft(TreeNode* root);
    static void printTreeHelp(TreeNode* root, int space);
};

///-------------------------GENERIC FUNCTIONS' IMPLEMENTATIONS-------------------------

//-------------------------AVL FUNCTIONS-------------------------

template <class KeyType, class DataType, class TreeNode>
AVL<KeyType, DataType, TreeNode>::AVL() : size(0) {
    dummyRoot = new TreeNode(KeyType(), DataType());
}

template <class KeyType, class DataType, class TreeNode>
AVL<KeyType, DataType, TreeNode>::~AVL() {
    if (size != 0) {
        TreeIterator iter = begin();
        auto ptr = iter.curr;
        TreeNode* last = nullptr;

        // PostOrder Traversal to delete all nodes
        while (ptr != dummyRoot) {
            while (ptr->left != nullptr) {
                ptr = ptr->left;
            }

            while (ptr->right != nullptr) {
                ptr = ptr->right;

                while (ptr->left != nullptr) {
                    ptr = ptr->left;
                }
            }

            last = ptr->parent;
            if (ptr->isLeftSubtree()) {
                last->left = nullptr;
            }
            else {
                last->right = nullptr;
            }
            delete ptr;

            ptr = last;
        }
    }

    delete dummyRoot;
}

template <class KeyType, class DataType, class TreeNode>
typename AVL<KeyType, DataType, TreeNode>::TreeIterator AVL<KeyType, DataType, TreeNode>::find(const KeyType& key) const {
    TreeNode* ptr = dummyRoot->left;

    while (ptr != nullptr && key != ptr->key) {
        if (key < ptr->key) {
            ptr = ptr->left;
        }
        else {
            ptr = ptr->right;
        }
    }

    auto iter = end(); // if it doesn't exist - return end()

    if (ptr != nullptr) {
        iter.last = ptr->parent;
        iter.curr = ptr;
    }
    return iter;
}

template <class KeyType, class DataType, class TreeNode>
AVLResult AVL<KeyType, DataType, TreeNode>::insert(const KeyType& key, const DataType& data) {
    if (size != 0) {
        auto last = dummyRoot;
        auto ptr = dummyRoot->left;

        // find where the new node should be placed
        while (ptr != nullptr && key != ptr->key) {
            last = ptr;
            if (key < ptr->key) {
                ptr = ptr->left;
            } else {
                ptr = ptr->right;
            }
        }

        if (ptr != nullptr)
            return AVL_ALREADY_EXIST;    // key is already in the tree

        // Add the new node:
        ptr = new TreeNode(key, data, last);
        if (key < last->key) {
            last->left = ptr;
        } else {
            last->right = ptr;
        }

        // fix the tree
        fixTree(ptr->parent);
    }
    else
    {
        // tree is empty
        dummyRoot->left = new TreeNode(key, data, dummyRoot);
    }

    size++;
    return AVL_SUCCESS;
}

template <class KeyType, class DataType, class TreeNode>
AVLResult AVL<KeyType, DataType, TreeNode>::remove(const KeyType& key) {
    if (size == 0)
        return AVL_SUCCESS;

    // look for the node
    TreeIterator iter = find(key);
    if (iter == end())
        return AVL_NOT_EXIST; // the key doesn't exist in the tree

    auto to_delete = iter.curr;
    if (to_delete->hasTwoSons()) {
        // get next node in the inorder traversal
        iter++;
        auto next = iter.curr;

        // swap the two nodes
        TreeNode::swap(*to_delete, *next);

        to_delete = next; // update pointer of node to be removed
    }

    TreeNode* son = nullptr;
    if (to_delete->hasSingleSon()) {
        // find which is the single son
        son = to_delete->left;
        if (son == nullptr) {
            son = to_delete->right;
        }

        // set the son's parent to be the removed node's parent
        son->parent = to_delete->parent;
    }

    // set parent's son (if it's leaf son = nullptr)
    if (to_delete->isLeftSubtree()) {
        to_delete->parent->left = son;
    }
    else {
        to_delete->parent->right = son;
    }

    auto to_fix = to_delete->parent;
    delete to_delete;
    fixTree(to_fix);

    size--; // update tree size

    return AVL_SUCCESS;
}

template <class KeyType, class DataType, class TreeNode>
typename AVL<KeyType, DataType, TreeNode>::TreeIterator AVL<KeyType, DataType, TreeNode>::begin() const {
    // go all the way left
    TreeIterator iter;
    iter.curr = dummyRoot;

    while (iter.curr->left != nullptr) {
        iter.curr = iter.curr->left;
    }
    iter.last = iter.curr->parent;

    return iter;
}

template <class KeyType, class DataType, class TreeNode>
typename AVL<KeyType, DataType, TreeNode>::TreeIterator AVL<KeyType, DataType, TreeNode>::Rbegin() const {
    // go all the way right
    TreeIterator iter;
    iter.curr = dummyRoot->left;

    while (iter.curr->right != nullptr) {
        iter.curr = iter.curr->right;
    }
    iter.last = iter.curr->parent;

    return iter;
}

template <class KeyType, class DataType, class TreeNode>
typename AVL<KeyType, DataType, TreeNode>::TreeIterator AVL<KeyType, DataType, TreeNode>::end() const {
    // dummy
    TreeIterator iter;
    iter.curr = dummyRoot;
    return iter;
}

template <class KeyType, class DataType, class TreeNode>
int AVL<KeyType, DataType, TreeNode>::getSize() const {
    return size;
}

template <class KeyType, class DataType, class TreeNode>
void AVL<KeyType, DataType, TreeNode>::printTree() {
    printTreeHelp(dummyRoot->left, 0);
}

//-------------------------PRIVATE AVL FUNCTIONS-------------------------

template <class KeyType, class DataType, class TreeNode>
void AVL<KeyType, DataType, TreeNode>::fixTree(TreeNode* root) {
    while (root != dummyRoot) {
        root->updateRanks();
        BalanceSubTree(root);
        root = root->parent;
    }
}

template <class KeyType, class DataType, class TreeNode>
void AVL<KeyType, DataType, TreeNode>::BalanceSubTree(TreeNode* root) {
    if (root == nullptr || root == dummyRoot)
        return;

    int BF = root->getBalanceFactor();
    if (BF == 2) {
        int BF_left = root->left->getBalanceFactor();
        if (BF_left >= 0) {
            // LL
            rotateRight(root);
        }
        else if (BF_left == -1) {
            // LR
            rotateLeft(root->left);
            rotateRight(root);
        }
    }
    else if (BF == -2) {
        int BF_right = root->right->getBalanceFactor();
        if (BF_right <= 0) {
            // RR
            rotateLeft(root);
        }
        else if (BF_right == 1) {
            // RL
            rotateRight(root->right);
            rotateLeft(root);
        }
    }
}

template <class KeyType, class DataType, class TreeNode>
void AVL<KeyType, DataType, TreeNode>::rotateRight(TreeNode* root) {
    if (root == nullptr || root == dummyRoot)
        return;

    // save relevant pointers
    auto parent = root->parent;
    auto B = root;
    auto A = root->left;
    auto A_R = A->right;

    // change pointers accordingly
    if (root->isLeftSubtree()) {
        parent->left = A;
    }
    else {
        parent->right = A;
    }
    A->parent = parent;

    B->left = A_R;
    if (A_R != nullptr)
        A_R->parent = B;

    A->right = B;
    B->parent = A;

    A->updateRanks();
    B->updateRanks();
}

template <class KeyType, class DataType, class TreeNode>
void AVL<KeyType, DataType, TreeNode>::rotateLeft(TreeNode* root) {
    if (root == nullptr || root == dummyRoot)
        return;

    // get relevant pointers
    auto parent = root->parent;
    auto A = root;
    auto B = root->right;
    auto B_L = B->left;

    // change pointers accordingly
    if (root->isLeftSubtree()) {
        parent->left = B;
    }
    else {
        parent->right = B;
    }
    B->parent = parent;

    A->right = B_L;
    if (B_L != nullptr)
        B_L->parent = A;

    B->left = A;
    A->parent = B;

    A->updateRanks();
    B->updateRanks();
}

template <class KeyType, class DataType, class TreeNode>
void AVL<KeyType, DataType, TreeNode>::printTreeHelp(TreeNode* root, int space) {
    // Base case
    if (root == nullptr)
        return;

    // Increase distance between levels
    space += COUNT;

    // Process right child first
    printTreeHelp(root->right, space);

    // Print current node after space
    // count
    cout << endl;
    for (int i = COUNT; i < space; i++)
        cout << " ";
    cout << root->key << "\n";

    // Process left child
    printTreeHelp(root->left, space);
}

//-------------------------AVL TREE ITERATOR FUNCTIONS-------------------------
template <class KeyType, class DataType, class TreeNode>
DataType& AVL<KeyType, DataType, TreeNode>::TreeIterator::operator*() const {
    // assert(curr->parent != nullptr); // can't dereference the dummy
    return (curr->data);
}

template <class KeyType, class DataType, class TreeNode>
const typename AVL<KeyType, DataType, TreeNode>::TreeIterator AVL<KeyType, DataType, TreeNode>::TreeIterator::operator++(int) {
    // check if reached end (dummyNode) before ++
    if (curr->parent == nullptr)
        return *this;

    // doSomething(curr) was done

    // if a right subtree exists
    if (curr->right != nullptr) {
        last = curr;
        curr = curr->right; // go right

        // and then go left as much as possible
        while (curr->left != nullptr) {
            last = curr;
            curr = curr->left;
        }
    }
    else {
        // no right subtree exists
        last = curr;
        curr = curr->parent; // go up

        // if you came back from a right subtree
        // keep rising until you come back from a left subtree
        while (last == curr->right) {
            last = curr;
            curr = curr->parent;
        }
    }

    return *this;   // doSomething(curr) will be done
}


template <class KeyType, class DataType, class TreeNode>
const typename AVL<KeyType, DataType, TreeNode>::TreeIterator AVL<KeyType, DataType, TreeNode>::TreeIterator::operator--(int) {
    // check if reached end (dummyNode) before ++
    if (curr->parent == nullptr)
        return *this;

    // doSomething(curr) was done

    // if a left subtree exists
    if (curr->left != nullptr) {
        last = curr;
        curr = curr->left; // go left

        // and then go right as much as possible
        while (curr->right != nullptr) {
            last = curr;
            curr = curr->right;
        }
    }
    else {
        // no left subtree exists
        last = curr;
        curr = curr->parent; // go up

        // if you came back from a left subtree
        // keep rising until you come back from a right subtree
        while (last == curr->left) {
            last = curr;
            curr = curr->parent;
        }
    }

    return *this;   // doSomething(curr) will be done
}


template <class KeyType, class DataType, class TreeNode>
bool AVL<KeyType, DataType, TreeNode>::TreeIterator::operator<(const TreeIterator& other) const {
    if (this->curr->parent == nullptr)
        return false; // if this is the end

    if (other.curr->parent == nullptr)
        return true; // everything is smaller than the end

    // compare keys with key's operator <
    return (curr->key < other.curr->key);
}

template <class KeyType, class DataType, class TreeNode>
bool AVL<KeyType, DataType, TreeNode>::TreeIterator::operator==(const TreeIterator& other) const {
    return (curr == other.curr);
}

template <class KeyType, class DataType, class TreeNode>
bool AVL<KeyType, DataType, TreeNode>::TreeIterator::operator!=(const TreeIterator& other) const {
    return !operator==(other);
}


//-------------------------TREE NODE FUNCTIONS-------------------------
template <class KeyType, class DataType>
int DefTreeNode<KeyType, DataType>::getBalanceFactor() const {
    int left_height = -1, right_height = -1;

    if (left != nullptr) {
        left_height = left->height;
    }
    if (right != nullptr) {
        right_height = right->height;
    }

    return (left_height - right_height);
}

template <class KeyType, class DataType>
bool DefTreeNode<KeyType, DataType>::isLeftSubtree() const {
    return (parent->left == this);
}

template <class KeyType, class DataType>
bool DefTreeNode<KeyType, DataType>::isLeaf() const {
    return (left == nullptr && right == nullptr);
}

template <class KeyType, class DataType>
bool DefTreeNode<KeyType, DataType>::hasTwoSons() const {
    return (left != nullptr && right != nullptr);
}

template <class KeyType, class DataType>
bool DefTreeNode<KeyType, DataType>::hasSingleSon() const {
    return (!isLeaf() && !hasTwoSons());
}

template<class KeyType, class DataType>
void DefTreeNode<KeyType, DataType>::updateRanks() {
    if (isLeaf()) {
        height = 0;
        return;
    }

    int left_height = -1, right_height = -1;

    if (left != nullptr) {
        left_height = left->height;
    }
    if (right != nullptr) {
        right_height = right->height;
    }

    height = (left_height > right_height) ? left_height : right_height;
    height++;
}

template<class KeyType, class DataType>
void DefTreeNode<KeyType, DataType>::swap(DefTreeNode<KeyType, DataType>& A, DefTreeNode<KeyType, DataType>& B) {
    KeyType temp_key = A.key;
    DataType temp_data = A.data;

    A.key = B.key;
    A.data = B.data;

    B.key = temp_key;
    B.data = temp_data;
}

#endif //DATACENTERS_WET1_AVL_H
