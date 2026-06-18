/**
 * 地铁线路图查询器（学生版）
 * 实验任务：完成所有标记为 // TODO 的函数实现。
 * 编译：gcc -o metro metro_student.c -std=c99
 * 运行：./metro
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAX_NAME_LEN 32
#define MAX_LINE_NAME 20

// 邻接表边结点
typedef struct EdgeNode {
    int adjVex;               // 邻接站点编号
    int weight;               // 权值（运行时间，分钟）
    struct EdgeNode *next;
} EdgeNode;

// 顶点结点（站点）
typedef struct VertexNode {
    char name[MAX_NAME_LEN];  // 站点名称
    EdgeNode *firstEdge;      // 第一条边
    int *lineIds;             // 该站点所属的线路编号数组（动态分配）
    int lineCount;            // 所属线路数量
} VertexNode;

// 图结构
typedef struct {
    VertexNode *vertices;     // 顶点数组
    int vertexNum;            // 实际顶点数
    int vertexCapacity;       // 顶点数组容量
    int edgeNum;              // 边数
    int isDirected;           // 0:无向, 1:有向
} Graph;

// 辅助队列（用于BFS）
typedef struct Queue {
    int *data;
    int front, rear, size, capacity;
} Queue;

// 函数声明
Graph* createGraph(int initCapacity, int isDirected);
void resizeGraph(Graph *g);
int addVertex(Graph *g, const char *name);
int findVertexIndex(Graph *g, const char *name);
void addEdge(Graph *g, int u, int v, int weight);
void addLineToStation(Graph *g, int stationIdx, int lineId);
void readMetroFile(const char *filename, Graph *g);
void printAdjList(Graph *g);

// 以下是需要实现的函数（TODO）
void DFSRecursive(Graph *g, int v, int *visited);
void DFSTraversal(Graph *g, int start);
void BFSTraversal(Graph *g, int start);
void connectivityAnalysis(Graph *g);
void dijkstra(Graph *g, int start, int *dist, int *prev);
void printPath(Graph *g, int *prev, int start, int end);
void shortestPathByTime(Graph *g, int start, int end);
void shortestPathByTransfer(Graph *g, int start, int end);
void freeGraph(Graph *g);

void printMenu();

// 队列操作
Queue* createQueue(int capacity);
void enqueue(Queue *q, int val);
int dequeue(Queue *q);
int isEmpty(Queue *q);
void freeQueue(Queue *q);

// ---------- 主函数 ----------
int main() {
    Graph *g = createGraph(100, 0);  // 无向图

    readMetroFile("metro.txt", g);

    int choice, start, end;
    char startName[MAX_NAME_LEN], endName[MAX_NAME_LEN];

    do {
        printMenu();
        printf("请输入选择：");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                printAdjList(g);
                break;
            case 2:
                printf("请输入起始站点名称：");
                fgets(startName, MAX_NAME_LEN, stdin);
                startName[strcspn(startName, "\n")] = '\0';
                start = findVertexIndex(g, startName);
                if (start == -1) {
                    fprintf(stderr, "错误：站点 '%s' 不存在。\n", startName);
                } else {
                    printf("\nDFS 遍历序列（从 %s 开始）：\n", startName);
                    DFSTraversal(g, start);
                }
                break;
            case 3:
                printf("请输入起始站点名称：");
                fgets(startName, MAX_NAME_LEN, stdin);
                startName[strcspn(startName, "\n")] = '\0';
                start = findVertexIndex(g, startName);
                if (start == -1) {
                    fprintf(stderr, "错误：站点 '%s' 不存在。\n", startName);
                } else {
                    printf("\nBFS 遍历序列（从 %s 开始）：\n", startName);
                    BFSTraversal(g, start);
                }
                break;
            case 4:
                connectivityAnalysis(g);
                break;
            case 5:
                printf("请输入起点站：");
                fgets(startName, MAX_NAME_LEN, stdin);
                startName[strcspn(startName, "\n")] = '\0';
                printf("请输入终点站：");
                fgets(endName, MAX_NAME_LEN, stdin);
                endName[strcspn(endName, "\n")] = '\0';
                start = findVertexIndex(g, startName);
                end = findVertexIndex(g, endName);
                if (start == -1) {
                    fprintf(stderr, "错误：起点 '%s' 不存在。\n", startName);
                } else if (end == -1) {
                    fprintf(stderr, "错误：终点 '%s' 不存在。\n", endName);
                } else {
                    shortestPathByTime(g, start, end);
                }
                break;
            case 6:
                printf("请输入起点站：");
                fgets(startName, MAX_NAME_LEN, stdin);
                startName[strcspn(startName, "\n")] = '\0';
                printf("请输入终点站：");
                fgets(endName, MAX_NAME_LEN, stdin);
                endName[strcspn(endName, "\n")] = '\0';
                start = findVertexIndex(g, startName);
                end = findVertexIndex(g, endName);
                if (start == -1) {
                    fprintf(stderr, "错误：起点 '%s' 不存在。\n", startName);
                } else if (end == -1) {
                    fprintf(stderr, "错误：终点 '%s' 不存在。\n", endName);
                } else {
                    shortestPathByTransfer(g, start, end);
                }
                break;
            case 0:
                printf("退出程序。\n");
                break;
            default:
                printf("无效选择，请重新输入。\n");
        }
        printf("\n");
    } while (choice != 0);

    freeGraph(g);
    return 0;
}

// ---------- 以下函数已完整实现（无需修改）----------

// 创建图
Graph* createGraph(int initCapacity, int isDirected) {
    Graph *g = (Graph*)malloc(sizeof(Graph));
    g->vertexCapacity = initCapacity;
    g->vertexNum = 0;
    g->edgeNum = 0;
    g->isDirected = isDirected;
    g->vertices = (VertexNode*)malloc(sizeof(VertexNode) * initCapacity);
    for (int i = 0; i < initCapacity; i++) {
        g->vertices[i].name[0] = '\0';
        g->vertices[i].firstEdge = NULL;
        g->vertices[i].lineIds = NULL;
        g->vertices[i].lineCount = 0;
    }
    return g;
}

// 动态扩容
void resizeGraph(Graph *g) {
    int newCap = g->vertexCapacity * 2;
    g->vertices = (VertexNode*)realloc(g->vertices, sizeof(VertexNode) * newCap);
    for (int i = g->vertexCapacity; i < newCap; i++) {
        g->vertices[i].name[0] = '\0';
        g->vertices[i].firstEdge = NULL;
        g->vertices[i].lineIds = NULL;
        g->vertices[i].lineCount = 0;
    }
    g->vertexCapacity = newCap;
}

// 添加站点，返回编号
int addVertex(Graph *g, const char *name) {
    int idx = findVertexIndex(g, name);
    if (idx != -1) return idx;

    if (g->vertexNum >= g->vertexCapacity) {
        resizeGraph(g);
    }
    strcpy(g->vertices[g->vertexNum].name, name);
    g->vertices[g->vertexNum].firstEdge = NULL;
    g->vertices[g->vertexNum].lineIds = NULL;
    g->vertices[g->vertexNum].lineCount = 0;
    return g->vertexNum++;
}

// 查找站点编号
int findVertexIndex(Graph *g, const char *name) {
    for (int i = 0; i < g->vertexNum; i++) {
        if (strcmp(g->vertices[i].name, name) == 0)
            return i;
    }
    return -1;
}

// 添加边（无向图加双向）
void addEdge(Graph *g, int u, int v, int weight) {
    if (u < 0 || u >= g->vertexNum || v < 0 || v >= g->vertexNum) return;

    EdgeNode *e = (EdgeNode*)malloc(sizeof(EdgeNode));
    e->adjVex = v;
    e->weight = weight;
    e->next = g->vertices[u].firstEdge;
    g->vertices[u].firstEdge = e;

    if (!g->isDirected) {
        e = (EdgeNode*)malloc(sizeof(EdgeNode));
        e->adjVex = u;
        e->weight = weight;
        e->next = g->vertices[v].firstEdge;
        g->vertices[v].firstEdge = e;
    }
    g->edgeNum++;
}

// 为站点添加所属线路编号
void addLineToStation(Graph *g, int stationIdx, int lineId) {
    if (stationIdx < 0 || stationIdx >= g->vertexNum) return;
    for (int i = 0; i < g->vertices[stationIdx].lineCount; i++) {
        if (g->vertices[stationIdx].lineIds[i] == lineId)
            return;
    }
    g->vertices[stationIdx].lineCount++;
    g->vertices[stationIdx].lineIds = (int*)realloc(g->vertices[stationIdx].lineIds,
                                                    sizeof(int) * g->vertices[stationIdx].lineCount);
    g->vertices[stationIdx].lineIds[g->vertices[stationIdx].lineCount - 1] = lineId;
}

// 读取地铁文件
void readMetroFile(const char *filename, Graph *g) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "无法打开文件 %s\n", filename);
        exit(1);
    }

    char line[256];
    int routeCount = 0;
    // 跳过前两行（总站点数和线路数，这里简单处理：读取直到遇到第一条线路数据）
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        sscanf(line, "%d", &routeCount);
        break;
    }
    // 再读一行（线路数）
    fgets(line, sizeof(line), fp);

    for (int rid = 0; rid < routeCount; rid++) {
        if (!fgets(line, sizeof(line), fp)) break;
        if (line[0] == '#' || line[0] == '\n') {
            rid--;
            continue;
        }

        char lineName[MAX_LINE_NAME];
        int stationCount;
        char *token = strtok(line, " \t\n");
        if (!token) continue;
        strcpy(lineName, token);

        token = strtok(NULL, " \t\n");
        if (!token) continue;
        stationCount = atoi(token);

        int prevStation = -1;
        int timeVal = 1;
        for (int i = 0; i < stationCount; i++) {
            token = strtok(NULL, " \t\n");
            if (!token) break;

            // 判断是否为纯数字（时间）
            int isTime = 1;
            for (char *p = token; *p; p++) {
                if (!isdigit(*p)) { isTime = 0; break; }
            }
            if (isTime && i > 0) {
                timeVal = atoi(token);
                continue;
            }

            int idx = addVertex(g, token);
            addLineToStation(g, idx, rid);

            if (prevStation != -1) {
                addEdge(g, prevStation, idx, timeVal);
                timeVal = 1;
            }
            prevStation = idx;
        }
    }
    fclose(fp);
    printf("成功读取地铁数据：共 %d 个站点，%d 条边。\n", g->vertexNum, g->edgeNum);
}

// 输出邻接表及换乘站
void printAdjList(Graph *g) {
    printf("\n===== 邻接表 =====\n");
    for (int i = 0; i < g->vertexNum; i++) {
        printf("%s (%d条线路): ", g->vertices[i].name, g->vertices[i].lineCount);
        EdgeNode *e = g->vertices[i].firstEdge;
        while (e) {
            printf("-> %s(%dmin) ", g->vertices[e->adjVex].name, e->weight);
            e = e->next;
        }
        printf("\n");
    }
    printf("\n===== 换乘站 =====\n");
    for (int i = 0; i < g->vertexNum; i++) {
        if (g->vertices[i].lineCount > 1) {
            printf("%s:%d 条线路\n", g->vertices[i].name, g->vertices[i].lineCount);
        }
    }
}

// 打印菜单
void printMenu() {
    printf("\n====== 地铁查询系统 ======\n");
    printf("1. 输出邻接表和换乘站\n");
    printf("2. DFS 遍历（从指定站点）\n");
    printf("3. BFS 遍历（从指定站点）\n");
    printf("4. 连通分量分析\n");
    printf("5. 最短路径（最少时间）\n");
    printf("6. 最短路径（最少换乘）\n");
    printf("0. 退出\n");
}

// ---------- 队列实现（已提供）----------
Queue* createQueue(int capacity) {
    Queue *q = (Queue*)malloc(sizeof(Queue));
    q->data = (int*)malloc(sizeof(int) * capacity);
    q->front = q->rear = q->size = 0;
    q->capacity = capacity;
    return q;
}

void enqueue(Queue *q, int val) {
    if (q->size == q->capacity) return;
    q->data[q->rear] = val;
    q->rear = (q->rear + 1) % q->capacity;
    q->size++;
}

int dequeue(Queue *q) {
    if (q->size == 0) return -1;
    int val = q->data[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    return val;
}

int isEmpty(Queue *q) {
    return q->size == 0;
}

void freeQueue(Queue *q) {
    free(q->data);
    free(q);
}

// ---------- 以下为需要实现的函数（TODO）----------

void DFSRecursive(Graph *g, int v, int *visited) {
    if (v < 0 || v >= g->vertexNum || visited[v]) return;
    
    // 标记已访问并输出当前站点
    visited[v] = 1;
    printf("%s ", g->vertices[v].name);
    
    // 遍历所有邻接顶点
    EdgeNode *p = g->vertices[v].firstEdge;
    while (p != NULL) {
        if (!visited[p->adjVex]) {
            DFSRecursive(g, p->adjVex, visited);
        }
        p = p->next;
    }
} // TODO: 实现递归深度优先遍历

void DFSTraversal(Graph *g, int start) {
    if (g == NULL || start < 0 || start >= g->vertexNum) return;
    
    // 分配并初始化访问数组
    int *visited = (int*)malloc(sizeof(int) * g->vertexNum);
    memset(visited, 0, sizeof(int) * g->vertexNum);
    
    // 调用递归遍历
    DFSRecursive(g, start, visited);
    printf("\n");
    
    // 释放内存
    free(visited); // TODO: 调用 DFSRecursive 从 start 开始遍历并输出序列
}

void BFSTraversal(Graph *g, int start) {
    if (g == NULL || start < 0 || start >= g->vertexNum) return;
    
    // 初始化访问数组
    int *visited = (int*)malloc(sizeof(int) * g->vertexNum);
    memset(visited, 0, sizeof(int) * g->vertexNum);
    
    // 创建队列（容量为顶点数）
    Queue *q = createQueue(g->vertexNum);
    
    // 起始顶点入队
    visited[start] = 1;
    enqueue(q, start);
    printf("%s ", g->vertices[start].name);
    
    while (!isEmpty(q)) {
        // 出队当前顶点
        int v = dequeue(q);
        
        // 遍历所有邻接顶点
        EdgeNode *p = g->vertices[v].firstEdge;
        while (p != NULL) {
            if (!visited[p->adjVex]) {
                visited[p->adjVex] = 1;
                printf("%s ", g->vertices[p->adjVex].name);
                enqueue(q, p->adjVex);
            }
            p = p->next;
        }
    }
    printf("\n");// TODO: 使用队列实现广度优先遍历，输出序列
}

void connectivityAnalysis(Graph *g) {
   if (g == NULL) return;
    
    int *visited = (int*)malloc(sizeof(int) * g->vertexNum);
    memset(visited, 0, sizeof(int) * g->vertexNum);
    
    int componentCount = 0;
    printf("\n===== 连通分量分析 =====\n");
    
    for (int i = 0; i < g->vertexNum; i++) {
        if (!visited[i]) {
            componentCount++;
            printf("连通分量 %d:", componentCount);
            
            // BFS遍历当前连通分量
            Queue *q = createQueue(g->vertexNum);
            visited[i] = 1;
            enqueue(q, i);
            printf("%s ", g->vertices[i].name);
            
            while (!isEmpty(q)) {
                int v = dequeue(q);
                EdgeNode *p = g->vertices[v].firstEdge;
                while (p != NULL) {
                    if (!visited[p->adjVex]) {
                        visited[p->adjVex] = 1;
                        printf("%s ", g->vertices[p->adjVex].name);
                        enqueue(q, p->adjVex);
                    }
                    p = p->next;
                }
            }
            printf("\n");
            freeQueue(q);
        }
    }
    
    printf("总连通分量数：%d\n", componentCount);
    free(visited);  
}

void dijkstra(Graph *g, int start, int *dist, int *prev) {
   if (g == NULL || start < 0 || start >= g->vertexNum) return;
    
    int vNum = g->vertexNum;
    int *visited = (int*)malloc(sizeof(int) * vNum);
    
    // 初始化
    for (int i = 0; i < vNum; i++) {
        dist[i] = INT_MAX;
        prev[i] = -1;
        visited[i] = 0;
    }
    dist[start] = 0;
    
    for (int i = 0; i < vNum; i++) {
        // 找到未访问的距离最小顶点u
        int u = -1;
        int minDist = INT_MAX;
        for (int j = 0; j < vNum; j++) {
            if (!visited[j] && dist[j] < minDist) {
                minDist = dist[j];
                u = j;
            }
        }
        
        // 所有顶点已处理或不可达
        if (u == -1) break;
        visited[u] = 1;
        
        // 松弛操作
        EdgeNode *p = g->vertices[u].firstEdge;
        while (p != NULL) {
            int v = p->adjVex;
            if (!visited[v] && dist[u] != INT_MAX && dist[u] + p->weight < dist[v]) {
                dist[v] = dist[u] + p->weight;
                prev[v] = u;
            }
            p = p->next;
        }
    }
    
    free(visited);  // TODO: 实现 Dijkstra 算法，计算最短距离和前驱数组
}

void printPath(Graph *g, int *prev, int start, int end) {
   if (g == NULL || prev == NULL || start < 0 || end < 0 || start >= g->vertexNum || end >= g->vertexNum) {
        return;
    }
    
    // 递归基：到达起点
    if (start == end) {
        printf("%s", g->vertices[start].name);
        return;
    }
    
    // 无路径
    if (prev[end] == -1) {
        printf("无法到达");
        return;
    }
    
    // 递归输出前驱路径
    printPath(g, prev, start, prev[end]);
    printf(" -> %s", g->vertices[end].name);
      // TODO: 递归输出从 start 到 end 的路径
}
    void shortestPathByTime(Graph *g, int start, int end){
       
// 先判断图和站点合法性
    if (g == NULL || start < 0 || end < 0 || start >= g->vertexNum || end >= g->vertexNum){
        printf("无效的站点编号\n");
        return;
    }
    if(start == end){
        printf("起点和终点相同，无需出行\n");
        return;
    }

    // 【关键】函数内部定义 dist、prev 数组
    int vNum = g->vertexNum;
    int *dist = (int*)malloc(sizeof(int) * vNum);
    int *prev = (int*)malloc(sizeof(int) * vNum);

    // 执行迪杰斯特拉，填充距离数组、前驱数组
    dijkstra(g, start, dist, prev);

    printf("\n最短时间路径（总时间 %d 分钟）：", dist[end]);
    printPath(g, prev, start, end);
    printf("\n");

    // 释放动态内存
    free(dist);
    free(prev);
 // TODO: 使用 dijkstra 输出最少时间路径及总时间
}

void shortestPathByTransfer(Graph *g, int start, int end) {
   if (g == NULL || start < 0 || end < 0 || start >= g->vertexNum || end >= g->vertexNum) {
        printf("无效的站点编号\n");
        return;
    }
    
    if (start == end) {
        printf("起点和终点相同，无需出行\n");
        return;
    }
    
    // 保存原始权重并临时修改为1
    int vNum = g->vertexNum;
    // 定义结构体保存原始边信息
    typedef struct {
        int u, v;
        int weight;
    } EdgeInfo;
    
    EdgeInfo *edgeInfos = (EdgeInfo*)malloc(sizeof(EdgeInfo) * g->edgeNum);
    int edgeCount = 0;
    
    // 遍历所有顶点，保存并修改边权重
    for (int u = 0; u < vNum; u++) {
        EdgeNode *p = g->vertices[u].firstEdge;
        while (p != NULL) {
            // 无向图只保存一次（u < v）避免重复
            if (u < p->adjVex) {
                edgeInfos[edgeCount].u = u;
                edgeInfos[edgeCount].v = p->adjVex;
                edgeInfos[edgeCount].weight = p->weight;
                edgeCount++;
            }
            // 修改权重为1
            p->weight = 1;
            p = p->next;
        }
    }
    
    // 执行Dijkstra算法
    int *dist = (int*)malloc(sizeof(int) * vNum);
    int *prev = (int*)malloc(sizeof(int) * vNum);
    dijkstra(g, start, dist, prev);
    
    // 输出结果
    int transferCount = dist[end] - 1;  // 换乘次数=边数-1
    printf("\n最少换乘路径（换乘次数 %d 次）：", transferCount);
    printPath(g, prev, start, end);
    printf("\n");
    
    // 恢复原始权重
    for (int i = 0; i < edgeCount; i++) {
        int u = edgeInfos[i].u;
        int v = edgeInfos[i].v;
        int weight = edgeInfos[i].weight;
        
        // 恢复u->v的边
        EdgeNode *p = g->vertices[u].firstEdge;
        while (p != NULL && p->adjVex != v) {
            p = p->next;
        }
        if (p != NULL) p->weight = weight;
        
        // 恢复v->u的边
        p = g->vertices[v].firstEdge;
        while (p != NULL && p->adjVex != u) {
            p = p->next;
        }
        if (p != NULL) p->weight = weight;
    }
    
    // 释放内存
    free(dist);
    free(prev);
    free(edgeInfos); // TODO: 将边权临时设为1，调用 dijkstra，输出最少换乘路径及换乘次数，然后恢复原权值
}

void freeGraph(Graph *g) {
   if (g == NULL) return;
    
    // 释放每个顶点的边链表和线路数组
    for (int i = 0; i < g->vertexNum; i++) {
        // 释放边链表
        EdgeNode *p = g->vertices[i].firstEdge;
        while (p != NULL) {
            EdgeNode *temp = p;
            p = p->next;
            free(temp);
        }
        
        // 释放线路数组
        if (g->vertices[i].lineIds != NULL) {
            free(g->vertices[i].lineIds);
        }
    }
    
    // 释放顶点数组
    free(g->vertices);
    
    // 释放图结构体
    free(g);   // TODO: 释放所有动态分配的内存（边结点、lineIds、顶点数组、图结构）
}
