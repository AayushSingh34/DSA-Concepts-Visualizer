#include <winsock2.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <deque>
#include <string>
#include <algorithm>

using namespace std;

string vecToJson(const vector<int> &v)
{
    stringstream ss;
    ss << "[";
    for (size_t i = 0; i < v.size(); ++i)
    {
        ss << v[i] << (i + 1 < v.size() ? "," : "");
    }
    ss << "]";
    return ss.str();
}

vector<int> parseNumbers(const string &str, vector<int> fallback, int maxCount = 20)
{
    if (str.empty())
        return fallback;
    vector<int> result;
    stringstream ss(str);
    string item;
    while (getline(ss, item, ',') && (int)result.size() < maxCount)
    {
        try
        {
            int val = stoi(item);
            if (val > 0 && val <= 999)
                result.push_back(val);
        }
        catch (...)
        {
        }
    }
    return result.empty() ? fallback : result;
}

string getQueryParam(const string &fullPath, const string &key)
{
    size_t qPos = fullPath.find('?');
    if (qPos == string::npos)
        return "";
    string query = fullPath.substr(qPos + 1);
    string target = key + "=";
    size_t pos = query.find(target);
    if (pos == string::npos)
        return "";
    size_t start = pos + target.size();
    size_t end = query.find('&', start);
    return query.substr(start, end == string::npos ? string::npos : end - start);
}

// 1. ARRAY: Bubble Sort
// Helper to append a single sorting step to JSON
void addSortStep(stringstream &ss, bool &first, const vector<int> &arr,
                 const vector<int> &comp, const vector<int> &sortedIdx,
                 bool swapped, const string &msg)
{
    if (!first)
        ss << ",";
    first = false;
    ss << "{\"array\":" << vecToJson(arr)
       << ",\"compare\":" << vecToJson(comp)
       << ",\"sorted\":" << vecToJson(sortedIdx)
       << ",\"swapped\":" << (swapped ? "true" : "false")
       << ",\"msg\":\"" << msg << "\"}";
}

// 1A. Bubble Sort
string getBubbleSortSteps(vector<int> arr)
{
    stringstream ss;
    ss << "[";
    bool first = true;
    int n = arr.size();
    vector<int> sortedIdx;
    addSortStep(ss, first, arr, {}, sortedIdx, false, "Initial Array loaded into C++");

    for (int i = 0; i < n - 1; ++i)
    {
        for (int j = 0; j < n - i - 1; ++j)
        {
            bool swapped = false;
            if (arr[j] > arr[j + 1])
            {
                swap(arr[j], arr[j + 1]);
                swapped = true;
            }
            addSortStep(ss, first, arr, {j, j + 1}, sortedIdx, swapped,
                        "Comparing " + to_string(arr[j]) + " & " + to_string(arr[j + 1]) +
                            (swapped ? " -> Swapped!" : " -> In order"));
        }
        sortedIdx.push_back(n - 1 - i);
    }
    sortedIdx.clear();
    for (int i = 0; i < n; ++i)
        sortedIdx.push_back(i);
    addSortStep(ss, first, arr, {}, sortedIdx, false, "Bubble Sort Complete!");
    ss << "]";
    return ss.str();
}

// 1B. Selection Sort
string getSelectionSortSteps(vector<int> arr)
{
    stringstream ss;
    ss << "[";
    bool first = true;
    int n = arr.size();
    vector<int> sortedIdx;
    addSortStep(ss, first, arr, {}, sortedIdx, false, "Initial Array loaded into C++");

    for (int i = 0; i < n - 1; ++i)
    {
        int minIdx = i;
        for (int j = i + 1; j < n; ++j)
        {
            addSortStep(ss, first, arr, {minIdx, j}, sortedIdx, false,
                        "Checking if " + to_string(arr[j]) + " < current min (" + to_string(arr[minIdx]) + ")");
            if (arr[j] < arr[minIdx])
                minIdx = j;
        }
        if (minIdx != i)
        {
            swap(arr[i], arr[minIdx]);
            addSortStep(ss, first, arr, {i, minIdx}, sortedIdx, true,
                        "Swapped min " + to_string(arr[i]) + " into position " + to_string(i));
        }
        sortedIdx.push_back(i);
    }
    sortedIdx.clear();
    for (int i = 0; i < n; ++i)
        sortedIdx.push_back(i);
    addSortStep(ss, first, arr, {}, sortedIdx, false, "Selection Sort Complete!");
    ss << "]";
    return ss.str();
}

// 1C. Merge Sort
void mergeHelper(vector<int> &arr, int l, int m, int r, stringstream &ss, bool &first)
{
    vector<int> L(arr.begin() + l, arr.begin() + m + 1);
    vector<int> R(arr.begin() + m + 1, arr.begin() + r + 1);
    size_t i = 0, j = 0;
    int k = l;

    while (i < L.size() && j < R.size())
    {
        if (L[i] <= R[j])
            arr[k] = L[i++];
        else
            arr[k] = R[j++];
        addSortStep(ss, first, arr, {k}, {}, true,
                    "Merged " + to_string(arr[k]) + " into index " + to_string(k));
        k++;
    }
    while (i < L.size())
    {
        arr[k] = L[i++];
        addSortStep(ss, first, arr, {k}, {}, true,
                    "Placed remaining " + to_string(arr[k]) + " at index " + to_string(k));
        k++;
    }
    while (j < R.size())
    {
        arr[k] = R[j++];
        addSortStep(ss, first, arr, {k}, {}, true,
                    "Placed remaining " + to_string(arr[k]) + " at index " + to_string(k));
        k++;
    }
}

void mergeSortRec(vector<int> &arr, int l, int r, stringstream &ss, bool &first)
{
    if (l >= r)
        return;
    int m = l + (r - l) / 2;
    mergeSortRec(arr, l, m, ss, first);
    mergeSortRec(arr, m + 1, r, ss, first);
    mergeHelper(arr, l, m, r, ss, first);
}

string getMergeSortSteps(vector<int> arr)
{
    stringstream ss;
    ss << "[";
    bool first = true;
    int n = arr.size();
    addSortStep(ss, first, arr, {}, {}, false, "Initial Array loaded into C++");
    mergeSortRec(arr, 0, n - 1, ss, first);
    vector<int> allSorted;
    for (int i = 0; i < n; ++i)
        allSorted.push_back(i);
    addSortStep(ss, first, arr, {}, allSorted, false, "Merge Sort Complete!");
    ss << "]";
    return ss.str();
}

// 1D. Quick Sort
int partitionHelper(vector<int> &arr, int low, int high, vector<int> &sortedIdx, stringstream &ss, bool &first)
{
    int pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; ++j)
    {
        addSortStep(ss, first, arr, {j, high}, sortedIdx, false,
                    "Comparing " + to_string(arr[j]) + " with Pivot (" + to_string(pivot) + ")");
        if (arr[j] < pivot)
        {
            i++;
            if (i != j)
            {
                swap(arr[i], arr[j]);
                addSortStep(ss, first, arr, {i, j}, sortedIdx, true,
                            "Swapped " + to_string(arr[i]) + " & " + to_string(arr[j]));
            }
        }
    }
    swap(arr[i + 1], arr[high]);
    sortedIdx.push_back(i + 1);
    addSortStep(ss, first, arr, {i + 1, high}, sortedIdx, true,
                "Placed Pivot " + to_string(pivot) + " at final index " + to_string(i + 1));
    return i + 1;
}

void quickSortRec(vector<int> &arr, int low, int high, vector<int> &sortedIdx, stringstream &ss, bool &first)
{
    if (low < high)
    {
        int pi = partitionHelper(arr, low, high, sortedIdx, ss, first);
        quickSortRec(arr, low, pi - 1, sortedIdx, ss, first);
        quickSortRec(arr, pi + 1, high, sortedIdx, ss, first);
    }
    else if (low == high)
    {
        sortedIdx.push_back(low);
    }
}

string getQuickSortSteps(vector<int> arr)
{
    stringstream ss;
    ss << "[";
    bool first = true;
    int n = arr.size();
    vector<int> sortedIdx;
    addSortStep(ss, first, arr, {}, sortedIdx, false, "Initial Array loaded into C++");
    quickSortRec(arr, 0, n - 1, sortedIdx, ss, first);
    sortedIdx.clear();
    for (int i = 0; i < n; ++i)
        sortedIdx.push_back(i);
    addSortStep(ss, first, arr, {}, sortedIdx, false, "Quick Sort Complete!");
    ss << "]";
    return ss.str();
}
// 2. GRAPH: BFS
string getBfsSteps(int startNode)
{
    if (startNode < 0 || startNode > 5)
        startNode = 0;
    vector<vector<int>> adj = {{1, 2}, {0, 3, 4}, {0, 4, 5}, {1}, {1, 2, 5}, {2, 4}};
    vector<bool> visited(6, false);
    queue<int> q;
    vector<int> order;
    visited[startNode] = true;
    q.push(startNode);

    stringstream ss;
    ss << "[";
    bool first = true;
    while (!q.empty())
    {
        int curr = q.front();
        q.pop();
        order.push_back(curr);
        for (int nb : adj[curr])
        {
            if (!visited[nb])
            {
                visited[nb] = true;
                q.push(nb);
            }
        }
        queue<int> tempQ = q;
        vector<int> qVec;
        while (!tempQ.empty())
        {
            qVec.push_back(tempQ.front());
            tempQ.pop();
        }

        if (!first)
            ss << ",";
        first = false;
        ss << "{\"current\":" << curr << ",\"visited\":" << vecToJson(order)
           << ",\"queue\":" << vecToJson(qVec)
           << ",\"msg\":\"Dequeued Node " << curr << " -> Added unvisited neighbors\"}";
    }
    ss << "]";
    return ss.str();
}

// 3. TREES (Complete Binary Tree & BST)
struct TreeNode
{
    int val, x, y;
    TreeNode *left, *right;
    TreeNode(int v) : val(v), x(0), y(0), left(nullptr), right(nullptr) {}
};

TreeNode *insertBST(TreeNode *root, int val)
{
    if (!root)
        return new TreeNode(val);
    if (val < root->val)
        root->left = insertBST(root->left, val);
    else if (val > root->val)
        root->right = insertBST(root->right, val);
    return root;
}

TreeNode *buildBinaryTree(const vector<int> &vals)
{
    if (vals.empty())
        return nullptr;
    TreeNode *root = new TreeNode(vals[0]);
    queue<TreeNode *> q;
    q.push(root);
    size_t i = 1;
    while (i < vals.size())
    {
        TreeNode *curr = q.front();
        q.pop();
        curr->left = new TreeNode(vals[i++]);
        q.push(curr->left);
        if (i < vals.size())
        {
            curr->right = new TreeNode(vals[i++]);
            q.push(curr->right);
        }
    }
    return root;
}

void assignCoords(TreeNode *root, int x, int y, int offset)
{
    if (!root)
        return;
    root->x = x;
    root->y = y;
    int nextOff = max(offset / 2, 36);
    assignCoords(root->left, x - offset, y + 70, nextOff);
    assignCoords(root->right, x + offset, y + 70, nextOff);
}

void collectTree(TreeNode *root, vector<TreeNode *> &nodes, vector<pair<TreeNode *, TreeNode *>> &edges)
{
    if (!root)
        return;
    nodes.push_back(root);
    if (root->left)
    {
        edges.push_back({root, root->left});
        collectTree(root->left, nodes, edges);
    }
    if (root->right)
    {
        edges.push_back({root, root->right});
        collectTree(root->right, nodes, edges);
    }
}

void inorder(TreeNode *r, vector<int> &out)
{
    if (!r)
        return;
    inorder(r->left, out);
    out.push_back(r->val);
    inorder(r->right, out);
}
void preorder(TreeNode *r, vector<int> &out)
{
    if (!r)
        return;
    out.push_back(r->val);
    preorder(r->left, out);
    preorder(r->right, out);
}

string getTreeSteps(const string &type, vector<int> vals)
{
    TreeNode *root = nullptr;
    vector<int> order;
    if (type == "bst")
    {
        for (int v : vals)
            root = insertBST(root, v);
        inorder(root, order);
    }
    else
    {
        root = buildBinaryTree(vals);
        preorder(root, order);
    }
    assignCoords(root, 350, 45, 160);
    vector<TreeNode *> nodes;
    vector<pair<TreeNode *, TreeNode *>> edges;
    collectTree(root, nodes, edges);

    stringstream ss;
    ss << "{\"nodes\":[";
    for (size_t i = 0; i < nodes.size(); ++i)
        ss << "{\"val\":" << nodes[i]->val << ",\"x\":" << nodes[i]->x << ",\"y\":" << nodes[i]->y << "}" << (i + 1 < nodes.size() ? "," : "");
    ss << "],\"edges\":[";
    for (size_t i = 0; i < edges.size(); ++i)
        ss << "{\"x1\":" << edges[i].first->x << ",\"y1\":" << edges[i].first->y
           << ",\"x2\":" << edges[i].second->x << ",\"y2\":" << edges[i].second->y << "}" << (i + 1 < edges.size() ? "," : "");
    ss << "],\"order\":" << vecToJson(order) << "}";
    return ss.str();
}

// 4. LINKED LISTS (Singly, Doubly, Circular, Doubly Circular)
string getLinkedListSteps(const string &type, vector<int> vals)
{
    stringstream ss;
    ss << "[";
    vector<int> list;
    bool first = true;

    for (size_t i = 0; i < vals.size(); ++i)
    {
        list.push_back(vals[i]);
        if (!first)
            ss << ",";
        first = false;

        string msg = "Inserted " + to_string(vals[i]);
        if (i == 0)
            msg += " as HEAD";
        else if (type == "singly")
            msg += " -> Linked previous node's NEXT to " + to_string(vals[i]);
        else if (type == "doubly")
            msg += " -> Linked NEXT & PREV pointers bidirectionally";
        else if (type == "circular")
            msg += " -> Updated TAIL.next back to HEAD (" + to_string(list[0]) + ")";
        else if (type == "dcircular")
            msg += " -> Linked TAIL.next to HEAD & HEAD.prev to TAIL";

        ss << "{\"nodes\":" << vecToJson(list) << ",\"active\":" << i << ",\"msg\":\"" << msg << "\"}";
    }

    // Add a traversal step at the end
    for (size_t i = 0; i < list.size(); ++i)
    {
        ss << ",{\"nodes\":" << vecToJson(list) << ",\"active\":" << i
           << ",\"msg\":\"Traversing node " << list[i] << " (Index " << i << ")\"}";
    }
    ss << "]";
    return ss.str();
}

// 5. QUEUES (Linear, Circular, Priority, Deque)
string getQueueSteps(const string &type, vector<int> vals)
{
    stringstream ss;
    ss << "[";
    bool first = true;

    if (type == "linear")
    {
        vector<int> q;
        for (int v : vals)
        {
            q.push_back(v);
            if (!first)
                ss << ",";
            first = false;
            ss << "{\"items\":" << vecToJson(q) << ",\"front\":0,\"rear\":" << (int)q.size() - 1
               << ",\"msg\":\"ENQUEUE " << v << " at REAR (FIFO)\"}";
        }
        int f = 0;
        while (f < (int)q.size())
        {
            int removed = q[f];
            q[f] = -1; // -1 marks empty/dequeued slot
            f++;
            ss << ",{\"items\":" << vecToJson(q) << ",\"front\":" << (f < (int)q.size() ? f : -1)
               << ",\"rear\":" << (int)q.size() - 1
               << ",\"msg\":\"DEQUEUE " << removed << " from FRONT (Notice empty space left behind!)\"}";
        }
    }
    else if (type == "circular")
    {
        int cap = max(6, min((int)vals.size(), 20));
        vector<int> ring(cap, -1);
        int front = -1, rear = -1;

        auto pushRing = [&](int v, string note)
        {
            if (front == -1)
                front = 0;
            rear = (rear + 1) % cap;
            ring[rear] = v;
            if (!first)
                ss << ",";
            first = false;
            ss << "{\"items\":" << vecToJson(ring) << ",\"front\":" << front << ",\"rear\":" << rear
               << ",\"msg\":\"ENQUEUE " << v << " at slot " << rear << note << "\"}";
        };
        auto popRing = [&]()
        {
            int rem = ring[front];
            ring[front] = -1;
            int oldF = front;
            if (front == rear)
            {
                front = rear = -1;
            }
            else
                front = (front + 1) % cap;
            ss << ",{\"items\":" << vecToJson(ring) << ",\"front\":" << front << ",\"rear\":" << rear
               << ",\"msg\":\"DEQUEUE " << rem << " from slot " << oldF << "\"}";
        };

        int limit = min((int)vals.size(), cap);
        for (int i = 0; i < limit; ++i)
            pushRing(vals[i], "");
        if (limit >= 2)
        {
            popRing();
            popRing();
        }
        pushRing(99, " (Wrapped around to reuse empty slot!)");
        if (limit >= 2)
            pushRing(88, " (Wrapped around!)");
    }
    else if (type == "priority")
    {
        priority_queue<int> pq;
        auto pqToVec = [](priority_queue<int> copy)
        {
            vector<int> res;
            while (!copy.empty())
            {
                res.push_back(copy.top());
                copy.pop();
            }
            return res;
        };
        for (int v : vals)
        {
            pq.push(v);
            if (!first)
                ss << ",";
            first = false;
            ss << "{\"items\":" << vecToJson(pqToVec(pq)) << ",\"front\":0,\"rear\":" << (int)pq.size() - 1
               << ",\"msg\":\"PUSH " << v << " -> Max-Heap places highest priority (" << pq.top() << ") at FRONT\"}";
        }
        while (!pq.empty())
        {
            int topVal = pq.top();
            pq.pop();
            ss << ",{\"items\":" << vecToJson(pqToVec(pq)) << ",\"front\":0,\"rear\":" << (int)pq.size() - 1
               << ",\"msg\":\"POP highest priority element: " << topVal << "\"}";
        }
    }
    else if (type == "deque")
    {
        deque<int> dq;
        for (size_t i = 0; i < vals.size(); ++i)
        {
            bool toFront = (i % 2 == 1);
            if (toFront)
                dq.push_front(vals[i]);
            else
                dq.push_back(vals[i]);
            vector<int> v(dq.begin(), dq.end());
            if (!first)
                ss << ",";
            first = false;
            ss << "{\"items\":" << vecToJson(v) << ",\"front\":0,\"rear\":" << (int)v.size() - 1
               << ",\"msg\":\"" << (toFront ? "PUSH_FRONT " : "PUSH_BACK ") << vals[i] << "\"}";
        }
        if (!dq.empty())
        {
            int f = dq.front();
            dq.pop_front();
            vector<int> v(dq.begin(), dq.end());
            ss << ",{\"items\":" << vecToJson(v) << ",\"front\":0,\"rear\":" << (int)v.size() - 1
               << ",\"msg\":\"POP_FRONT removed " << f << "\"}";
        }
        if (!dq.empty())
        {
            int b = dq.back();
            dq.pop_back();
            vector<int> v(dq.begin(), dq.end());
            ss << ",{\"items\":" << vecToJson(v) << ",\"front\":0,\"rear\":" << (int)v.size() - 1
               << ",\"msg\":\"POP_BACK removed " << b << "\"}";
        }
    }

    ss << "]";
    return ss.str();
}

string readFile(const string &path)
{
    ifstream file(path);
    if (!file.is_open())
        return "<h1>Error: public/index.html not found</h1>";
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

string makeResponse(const string &body, const string &contentType)
{
    stringstream ss;
    ss << "HTTP/1.1 200 OK\r\nContent-Type: " << contentType
       << "\r\nContent-Length: " << body.size() << "\r\nConnection: close\r\n\r\n"
       << body;
    return ss.str();
}

int main()
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);
    bind(server_fd, (sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 10);

    cout << "C++ DSA Visualizer running at http://localhost:8080\n";

    while (true)
    {
        SOCKET client = accept(server_fd, nullptr, nullptr);
        if (client == INVALID_SOCKET)
            continue;

        char buffer[4096] = {0};
        recv(client, buffer, sizeof(buffer) - 1, 0);
        stringstream req(buffer);
        string method, fullPath;
        req >> method >> fullPath;

        string route = fullPath.substr(0, fullPath.find('?'));
        string response;

        if (route == "/api/sort")
        {
            response = makeResponse(getBubbleSortSteps(parseNumbers(getQueryParam(fullPath, "data"), {45, 18, 72, 30, 90, 12, 55})), "application/json");
        }
        else if (route == "/api/bfs")
        {
            string s = getQueryParam(fullPath, "start");
            int startNode = 0;
            try
            {
                if (!s.empty())
                    startNode = stoi(s);
            }
            catch (...)
            {
            }
            response = makeResponse(getBfsSteps(startNode), "application/json");
        }
        else if (route == "/api/tree")
        {
            string type = getQueryParam(fullPath, "type");
            response = makeResponse(getTreeSteps(type, parseNumbers(getQueryParam(fullPath, "data"), {50, 30, 70, 20, 40, 60, 80})), "application/json");
        }
        else if (route == "/api/ll")
        {
            string type = getQueryParam(fullPath, "type");
            response = makeResponse(getLinkedListSteps(type, parseNumbers(getQueryParam(fullPath, "data"), {10, 20, 30, 40, 50}, 20)), "application/json");
        }
        else if (route == "/api/queue")
        {
            string type = getQueryParam(fullPath, "type");
            response = makeResponse(getQueueSteps(type, parseNumbers(getQueryParam(fullPath, "data"), {15, 40, 10, 60, 25}, 20)), "application/json");
        }
        else
        {
            response = makeResponse(readFile("public/index.html"), "text/html");
        }

        size_t totalSent = 0;
        while (totalSent < response.size())
        {
            int sent = send(client, response.c_str() + totalSent, response.size() - totalSent, 0);
            if (sent <= 0)
                break;
            totalSent += sent;
        }
        closesocket(client);
    }
    closesocket(server_fd);
    WSACleanup();
    return 0;
}