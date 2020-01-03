#include "ServerRankTree.h"

//-------------------- SERVER KEY IMPLEMENTATION --------------------

bool ServerKey::operator<(const ServerKey& other) const {
    if (traffic == other.traffic)
        return serverId < other.serverId;
    return traffic < other.traffic;
}

//-------------------- RANK TREE NODE IMPLEMENTATION --------------------

void RankTreeNode::updateRanks() {
    BaseNode::updateRanks();

    if (isLeaf()) {
        subTreeTraffic = ((Server) BaseNode::data).traffic;
        subTreeSize = 1;
        return;
    }

    RankTreeNode* left_node = ((RankTreeNode*)left);
    RankTreeNode* right_node = ((RankTreeNode*)right);

    int left_size = 0, right_size = 0;
    int left_traffic = 0, right_traffic = 0;

    if (left != nullptr) {
        left_size = left_node->subTreeSize;
        left_traffic = left_node->subTreeTraffic;
    }
    if (right != nullptr) {
        right_size = right_node->subTreeSize;
        right_traffic = right_node->subTreeTraffic;
    }

    subTreeSize = left_size + right_size + 1;
    subTreeTraffic = left_traffic + right_traffic + ((Server) BaseNode::data).traffic;
}

//-------------------- RANK TREE FUNCTIONS --------------------

ServerRankTree ServerRankTree::MergeRankTrees(const ServerRankTree& a, const ServerRankTree& b) {
    int newTreeSize = a.getSize()+b.getSize();

    // allocate two array of "Server" class in size of the two trees
    auto helperArray = new Server[newTreeSize];

    // do inorder on both trees and fill the array in ascending order
    auto aIter = a.begin(), bIter = b.begin();
    int i=0;
    for (; aIter != a.end() && bIter != b.end(); i++) {
        if (aIter < bIter) {
            helperArray[i] = *aIter;
        } else {
            helperArray[i] = *bIter;
        }
    }

    for (; aIter != a.end(); i++) helperArray[i] = *aIter;
    for (; bIter != b.end(); i++) helperArray[i] = *bIter;

    // call MakeEmptyTree
    auto newTree = MakeEmptyTree(newTreeSize);

    // do inorder and fill the empty tree
    auto iter = newTree.begin();
    for (int j=0; iter != newTree.end(); j++, iter++) {
        // init key to insert
        auto key = ServerKey(helperArray[j].traffic, helperArray[j].serverID);

        // insert key & data
        iter.curr->data = helperArray[j];
        iter.curr->key = key;
    }

    // call InitRanks
    newTree.InitRanks();

    // free the array
    free(helperArray);

    // return tree
    return newTree;
}

int ServerRankTree::SumHighestTrafficServers(int k) {
    // if tree size <= k return all tree traffic
    if (getSize() <= k) return ((RankTreeNode*)dummyRoot->left)->subTreeTraffic;

    int trafficSum = 0;
    auto curr = dummyRoot->left;
    while (k > 0) {
        if ( ((RankTreeNode*)curr->right)->subTreeSize >= k ) {
            // no need to add to trafficSum because curr and curr's leftsubtree not counted
            // no need to decrement k for the same reason
            // go right
            curr = curr->right;

        } else {
            // add curr's and curr's rightsubtree traffic
            // decrement k by curr's rightsubtree size + 1
            // go left
            trafficSum += curr->data.traffic;
            trafficSum += ((RankTreeNode*)curr->right)->subTreeTraffic;
            k -= ( ((RankTreeNode*)curr->right)->subTreeSize + 1 );
            curr = curr->left;
        }
    }

    return trafficSum;
}

ServerRankTree ServerRankTree::MakeEmptyTree(int size) {
    // make skeleton of empty tree in proper size
    int newTreeHeight = log(size);
    RankTreeNode* root = MakeEmptyTreeHelp(newTreeHeight);

    // construct new tree and init dummy->left = root;
    ServerRankTree newTree;
    newTree.dummyRoot->left = root;

    // go backward inorder, and delete leaves until the skeleton size reaches the wanted size
    auto iter = newTree.Rbegin();
    for (int newTreeSize = pow(2, newTreeHeight+1) - 1; newTreeSize > size; newTreeSize--) {
        // go to the next leaf
        while ( !iter.curr->isLeaf() ) iter--;
        auto to_delete = iter.curr;
        iter--;

        // remove pointer from parent
        if (to_delete->isLeftSubtree()) {
            to_delete->parent->left = nullptr;
        } else {
            to_delete->parent->right = nullptr;
        }

        // free node
        free(to_delete);
    }

    // init size of tree to "size"
    newTree.size = size;

    return newTree;
}

RankTreeNode* ServerRankTree::MakeEmptyTreeHelp(int height) {
    if (height < 0) return nullptr;

    // create new RankTreeNode with "garbage" values
    auto newNode = new RankTreeNode(ServerKey(0,0) ,Server());

    // continue recursively
    newNode->left = MakeEmptyTreeHelp(height-1);
    newNode->right = MakeEmptyTreeHelp(height-1);

    return newNode;
}

void ServerRankTree::InitRanks() {
    InitRanksHelp( (RankTreeNode*)dummyRoot->left );
}

void ServerRankTree::InitRanksHelp(RankTreeNode* curr) {
    if (!curr) return;

    // postorder
    InitRanksHelp((RankTreeNode*)curr->left);
    InitRanksHelp((RankTreeNode*)curr->right);

    ((RankTreeNode*)curr)->updateRanks();
}

int ServerRankTree::log(int n) {
    /*
        wanted results:
        1->0
        2->1
        3->1
        4->2
        5->2
        6->2
        7->2
        8->3
        ...
    */

    int res = 0;

    while (n > 1) {
        n /= 2;
        res++;
    }

    return res;
}

int ServerRankTree::pow(int base, int power) {
    int res = 1;
    for (; power > 0; power--) res *= base;

    return res;
}