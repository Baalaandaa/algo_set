#include <cstddef>
#include <algorithm>
#include <iterator>

template<typename T>
class Set {

    struct Node {
        Node *parent;
        Node *left;
        Node *right;
        T key;
        size_t size;
        size_t height;

        Node() : parent(nullptr), left(nullptr), right(nullptr), key(T()), size(1), height(0) {}

        explicit Node(const T &key_) : parent(nullptr), left(nullptr), right(nullptr), key(key_), size(1), height(0) {}

        Node(const T &key_, Node *parent_) : parent(parent_), left(nullptr), right(nullptr), key(key_), size(1),
                                             height(0) {}

        ~Node() {
            parent = nullptr;
            delete left;
            delete right;
        }

    };

    using NodePtr = Node *;

public:

    template<typename ValueType>
    struct Iterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = ValueType;
        using pointer = ValueType *;
        using reference = ValueType &;

        explicit Iterator(NodePtr ptr = nullptr) : mPtr(ptr), isEnd(ptr == nullptr) {}

        Iterator(NodePtr ptr, bool isEnd_) : mPtr(ptr), isEnd(isEnd_) {}

        Iterator(const Iterator<ValueType> &rawIterator) : mPtr(rawIterator.mPtr), isEnd(rawIterator.isEnd) {}

        ~Iterator() {}

        reference operator*() {
            return mPtr->key;
        }

        pointer operator->() {
            return &mPtr->key;
        }

        Iterator<ValueType> operator++() {
            if (isEnd) {
                return (*this);
            }
            mPtr = GetNext(mPtr, isEnd);
            return (*this);
        }

        Iterator<ValueType> operator++(int) {
            if (isEnd) {
                return (*this);
            }
            auto temp(*this);
            mPtr = GetNext(mPtr, isEnd);
            return temp;
        }

        Iterator<ValueType> operator--() {
            if (isEnd) {
                mPtr = GetMaximum(mPtr);
                if (mPtr != nullptr) {
                    isEnd = false;
                }
            } else {
                mPtr = GetPrevious(mPtr, isEnd);
            }
            return (*this);
        }

        Iterator<ValueType> operator--(int) {
            auto temp(*this);
            if (isEnd) {
                mPtr = GetMaximum(mPtr);
                if (mPtr != nullptr) {
                    isEnd = false;
                }
            } else {
                mPtr = GetPrevious(mPtr, isEnd);
            }
            return temp;
        }

        friend bool operator==(const Iterator<ValueType> &lhs, const Iterator<ValueType> &rhs) {
            return (lhs.mPtr == rhs.mPtr && lhs.isEnd == rhs.isEnd);
        }

        friend bool operator!=(const Iterator<ValueType> &lhs, const Iterator<ValueType> &rhs) {
            return (lhs.mPtr != rhs.mPtr || lhs.isEnd != rhs.isEnd);
        }

    private:
        NodePtr mPtr;
        bool isEnd;
    };

    using iterator = Iterator<const T>;
    using const_iterator = Iterator<const T>;

    Set() : root(nullptr) {}

    template<typename Iterator>
    Set(Iterator begin, Iterator end) {
        root = nullptr;
        for (Iterator curr = begin; curr != end; ++curr) {
            insert(*curr);
        }
    }

    Set(std::initializer_list<T> elems) {
        root = nullptr;
        for (const T &key: elems) {
            insert(key);
        }
    }

    Set(const Set<T> &oth) {
        root = nullptr;
        for (const T &key: oth) {
            insert(key);
        }
    }

    Set<T> &operator=(const Set<T> &oth) {
        if (oth.root == root) {
            return (*this);
        }
        if (root != nullptr) {
            delete root;
        }
        root = nullptr;
        for (const T &key: oth) {
            insert(key);
        }
        return (*this);
    }

    ~Set() {
        if (root != nullptr) {
            delete root;
        }
    }

    void insert(const T &key) {
        Insert(root, key);
    }

    void erase(const T &key) {
        Erase(root, key);
    }

    size_t size() const {
        return Size(root);
    }

    bool empty() const {
        return root == nullptr;
    }

    const_iterator begin() const {
        NodePtr minimalNode = GetMinimum(root);
        return const_iterator(minimalNode, empty());
    }

    const_iterator end() const {
        return const_iterator(root, true);
    }

    iterator find(const T &key) const {
        NodePtr node = Find(root, key);
        if (node != nullptr) {
            return iterator(node, false);
        } else {
            return end();
        }
    }


    iterator lower_bound(const T &key) const {
        NodePtr node = LowerBound(root, key);
        if (node != nullptr) {
            return iterator(node, false);
        } else {
            return end();
        }
    }

private:

    size_t Height(const NodePtr node) const {
        if (node != nullptr) {
            return node->height;
        }
        return 0;
    }

    size_t Size(const NodePtr node) const {
        if (node != nullptr) {
            return node->size;
        }
        return 0;
    }

    int BalanceFactor(const NodePtr node) const {
        if (node == nullptr) {
            return 0;
        }
        return Height(node->left) - Height(node->right);
    }

    void Pull(NodePtr node) {
        if (node == nullptr) {
            return;
        }
        node->height = 0;
        node->size = 1;
        if (node->left != nullptr) {
            node->height = std::max(node->height, node->left->height + 1);
            node->size += node->left->size;
        }
        if (node->right != nullptr) {
            node->height = std::max(node->height, node->right->height + 1);
            node->size += node->right->size;
        }
    }

    void RotateRight(NodePtr &node) {
        NodePtr newNode = node->right;
        node->right = newNode->left;
        newNode->left = node;

        newNode->parent = node->parent;
        if (newNode->left != nullptr) {
            newNode->left->parent = newNode;
        }
        if (node->right != nullptr) {
            node->right->parent = node;
        }

        Pull(newNode->left);
        Pull(newNode);

        node = newNode;
    }

    void RotateLeft(NodePtr &node) {
        NodePtr newNode = node->left;
        node->left = newNode->right;
        newNode->right = node;

        newNode->parent = node->parent;
        if (newNode->right != nullptr) {
            newNode->right->parent = newNode;
        }
        if (node->left != nullptr) {
            node->left->parent = node;
        }

        Pull(newNode->right);
        Pull(newNode);

        node = newNode;
    }

    void Rebalance(NodePtr &node) {
        if (BalanceFactor(node) < -1) {
            if (BalanceFactor(node->right) > 0) {
                RotateLeft(node->right);
            }
            RotateRight(node);
        } else if (BalanceFactor(node) > 1) {
            if (BalanceFactor(node->left) < 0) {
                RotateRight(node->left);
            }
            RotateLeft(node);
        }
        Pull(node);
    }

    void Insert(NodePtr &node, const T &key, NodePtr parent = nullptr) {
        if (node == nullptr) {
            node = new Node(key, parent);
            return;
        }
        if (!(key < node->key) && !(node->key < key)) {
            return;
        }
        if (key < node->key) {
            Insert(node->left, key, node);
        } else {
            Insert(node->right, key, node);
        }
        Rebalance(node);
    }

    static NodePtr GetMinimum(NodePtr node) {
        if (node == nullptr) {
            return nullptr;
        }
        if (node->left != nullptr) {
            return GetMinimum(node->left);
        }
        return node;
    }

    static NodePtr GetMaximum(NodePtr node) {
        if (node == nullptr) {
            return nullptr;
        }
        if (node->right != nullptr) {
            return GetMaximum(node->right);
        }
        return node;
    }

    static NodePtr GetNext(NodePtr node, bool &isEnd) {
        if (node == nullptr) {
            return node;
        }
        isEnd = false;
        if (node->right == nullptr) {
            while (node->parent != nullptr && node->parent->right == node) {
                node = node->parent;
            }
            if (node->parent == nullptr) {
                isEnd = true;
                return node;
            }
            return node->parent;
        } else {
            return GetMinimum(node->right);
        }
    }

    static NodePtr GetPrevious(NodePtr node, bool &isEnd) {
        if (node == nullptr) {
            return node;
        }
        isEnd = false;
        NodePtr original = node;
        if (node->left == nullptr) {
            while (node->parent != nullptr && node->parent->left == node) {
                node = node->parent;
            }
            if (node->parent == nullptr) {
                return original;
            }
            return node->parent;
        } else {
            return GetMaximum(node->left);
        }
    }

    NodePtr EraseMinimum(NodePtr node, NodePtr parent = nullptr) {
        if (node->left == nullptr) {
            NodePtr res = node->right;
            node->right = nullptr;
            node->parent = nullptr;
            return res;
        } else {
            node->left = EraseMinimum(node->left);
            if (node->left != nullptr) {
                node->left->parent = node;
            }
            Pull(node->left);
            Rebalance(node);
            return node;
        }
    }

    void Erase(NodePtr &node, const T &key, NodePtr parent = nullptr) {
        if (node == nullptr) {
            return;
        }
        if (key < node->key) {
            Erase(node->left, key, node);
        } else if (node->key < key) {
            Erase(node->right, key, node);
        } else {
            NodePtr left = node->left;
            NodePtr right = node->right;
            if (left == nullptr && right == nullptr) {
                delete node;
                node = nullptr;
            } else if (right == nullptr) {
                left->parent = parent;
                node->left = nullptr;
                node->parent = nullptr;
                delete node;
                node = left;
            } else if (left == nullptr) {
                right->parent = parent;
                node->right = nullptr;
                node->parent = nullptr;
                delete node;
                node = right;
            } else {
                NodePtr newNode = GetMinimum(right);
                newNode->right = EraseMinimum(right);
                if (newNode->right != nullptr) {
                    newNode->right->parent = newNode;
                }
                newNode->left = left;
                newNode->left->parent = newNode;
                newNode->parent = parent;

                node->left = nullptr;
                node->right = nullptr;
                node->parent = nullptr;
                delete node;

                Rebalance(newNode);
                node = newNode;
            }
        }
        Rebalance(node);
    }

    NodePtr Find(NodePtr node, const T &key) const {
        if (node == nullptr) {
            return nullptr;
        }
        if (key < node->key) {
            return Find(node->left, key);
        } else if (node->key < key) {
            return Find(node->right, key);
        } else {
            return node;
        }
    }

    NodePtr LowerBound(NodePtr node, const T &key) const {
        if (node == nullptr) {
            return nullptr;
        }
        if (key < node->key) {
            NodePtr result = LowerBound(node->left, key);
            return (result == nullptr ? node : result);
        } else if (node->key < key) {
            return LowerBound(node->right, key);
        } else {
            return node;
        }
    }

    NodePtr root;

};