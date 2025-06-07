#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// --- Definições Globais e Estruturas ---

#define MAX_ROWS 10 // Tamanho máximo de linhas do labirinto
#define MAX_COLS 10 // Tamanho máximo de colunas do labirinto
#define MAX_NODES (MAX_ROWS * MAX_COLS) // Número máximo de nós no grafo

// Estrutura para representar uma célula (posição) no labirinto
typedef struct {
    int row;
    int col;
} Cell;

// Estrutura para um nó na lista de adjacência
typedef struct AdjListNode {
    int dest; // Índice do nó de destino
    struct AdjListNode* next;
} AdjListNode;

// Estrutura para o Grafo (Lista de Adjacência)
typedef struct Graph {
    int num_nodes;
    AdjListNode** adj_lists; // Array de ponteiros para listas de adjacência
} Graph;

// Estrutura para um nó da Fila (usado no BFS)
typedef struct QueueNode {
    int data; // Índice do nó
    struct QueueNode* next;
} QueueNode;

// Estrutura da Fila
typedef struct Queue {
    QueueNode *front, *rear;
} Queue;

// --- Funções Auxiliares de Conversão ---

// Converte coordenadas (linha, coluna) para um índice único do nó
int map_coord_to_index(int r, int c, int num_cols) {
    return r * num_cols + c;
}

// Converte um índice de nó para coordenadas (linha, coluna)
void map_index_to_coord(int index, int num_cols, Cell* cell) {
    cell->row = index / num_cols;
    cell->col = index % num_cols;
}

// Verifica se uma célula está dentro dos limites do labirinto
bool is_valid(int r, int c, int num_rows, int num_cols) {
    return (r >= 0 && r < num_rows && c >= 0 && c < num_cols);
}

// --- Funções da Fila (para BFS) ---

// Cria uma nova fila vazia
Queue* create_queue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (!q) {
        perror("Erro ao alocar fila");
        exit(EXIT_FAILURE);
    }
    q->front = q->rear = NULL;
    return q;
}

// Adiciona um elemento à fila
void enqueue(Queue* q, int data) {
    QueueNode* new_node = (QueueNode*)malloc(sizeof(QueueNode));
    if (!new_node) {
        perror("Erro ao alocar QueueNode");
        exit(EXIT_FAILURE);
    }
    new_node->data = data;
    new_node->next = NULL;
    if (q->rear == NULL) { // Fila vazia
        q->front = q->rear = new_node;
        return;
    }
    q->rear->next = new_node;
    q->rear = new_node;
}

// Remove e retorna o primeiro elemento da fila
int dequeue(Queue* q) {
    if (q->front == NULL) { // Fila vazia
        return -1; // Ou outro valor que indique erro/vazio
    }
    QueueNode* temp = q->front;
    int data = temp->data;
    q->front = q->front->next;
    if (q->front == NULL) { // Se a fila ficou vazia
        q->rear = NULL;
    }
    free(temp);
    return data;
}

// Verifica se a fila está vazia
bool is_empty_queue(Queue* q) {
    return q->front == NULL;
}

// Libera a memória da fila
void free_queue(Queue* q) {
    while (!is_empty_queue(q)) {
        dequeue(q); // Apenas chama para liberar os nós
    }
    free(q);
}

// --- Funções do Grafo ---

// Cria um novo nó da lista de adjacência
AdjListNode* create_adj_list_node(int dest) {
    AdjListNode* new_node = (AdjListNode*)malloc(sizeof(AdjListNode));
    if (!new_node) {
        perror("Erro ao alocar AdjListNode");
        exit(EXIT_FAILURE);
    }
    new_node->dest = dest;
    new_node->next = NULL;
    return new_node;
}

// Cria um grafo com 'num_nodes' nós
Graph* create_graph(int num_nodes) {
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    if (!graph) {
        perror("Erro ao alocar Grafo");
        exit(EXIT_FAILURE);
    }
    graph->num_nodes = num_nodes;
    graph->adj_lists = (AdjListNode**)malloc(num_nodes * sizeof(AdjListNode*));
    if (!graph->adj_lists) {
        perror("Erro ao alocar lista de adjacência");
        free(graph);
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < num_nodes; i++) {
        graph->adj_lists[i] = NULL;
    }
    return graph;
}

// Adiciona uma aresta ao grafo (de src para dest)
void add_edge(Graph* graph, int src, int dest) {
    // Adiciona dest à lista de src
    AdjListNode* new_node = create_adj_list_node(dest);
    new_node->next = graph->adj_lists[src];
    graph->adj_lists[src] = new_node;

    // Para um labirinto, as arestas são bidirecionais
    new_node = create_adj_list_node(src);
    new_node->next = graph->adj_lists[dest];
    graph->adj_lists[dest] = new_node;
}

// Libera a memória do grafo
void free_graph(Graph* graph) {
    if (!graph) return;
    for (int i = 0; i < graph->num_nodes; i++) {
        AdjListNode* current = graph->adj_lists[i];
        while (current) {
            AdjListNode* temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(graph->adj_lists);
    free(graph);
}

// --- Funções de Navegação (BFS e DFS) ---

// Imprime o caminho encontrado do início ao fim
void print_path(int parent[], int start_node, int end_node, int num_cols) {
    if (end_node == -1) {
        printf("Nenhum caminho encontrado.\n");
        return;
    }

    // Constrói o caminho de trás para frente
    int path[MAX_NODES];
    int path_len = 0;
    int current = end_node;

    while (current != -1 && current != start_node) {
        path[path_len++] = current;
        current = parent[current];
    }
    path[path_len++] = start_node; // Adiciona o nó inicial

    printf("Caminho encontrado:\n");
    for (int i = path_len - 1; i >= 0; i--) {
        Cell cell;
        map_index_to_coord(path[i], num_cols, &cell);
        printf("(%d, %d)", cell.row, cell.col);
        if (i > 0) {
            printf(" -> ");
        }
    }
    printf("\n");
}

/**
 * @brief Realiza uma Busca em Largura (BFS) para encontrar o caminho mais curto.
 *
 * @param graph O grafo que representa o labirinto.
 * @param start_node O índice do nó de partida.
 * @param end_node O índice do nó de chegada.
 * @param num_rows Número de linhas do labirinto.
 * @param num_cols Número de colunas do labirinto.
 */
void bfs(Graph* graph, int start_node, int end_node, int num_rows, int num_cols) {
    printf("\n--- Iniciando Busca em Largura (BFS) ---\n");

    bool visited[graph->num_nodes];
    int parent[graph->num_nodes]; // Para reconstruir o caminho

    for (int i = 0; i < graph->num_nodes; i++) {
        visited[i] = false;
        parent[i] = -1; // -1 indica nenhum pai
    }

    Queue* q = create_queue();
    enqueue(q, start_node);
    visited[start_node] = true;

    int path_found_end_node = -1;

    while (!is_empty_queue(q)) {
        int u = dequeue(q);

        if (u == end_node) {
            path_found_end_node = u;
            break; // Caminho mais curto encontrado
        }

        AdjListNode* temp = graph->adj_lists[u];
        while (temp) {
            int v = temp->dest;
            if (!visited[v]) {
                visited[v] = true;
                parent[v] = u;
                enqueue(q, v);
            }
            temp = temp->next;
        }
    }

    if (path_found_end_node != -1) {
        printf("Caminho encontrado por BFS (mais curto):\n");
        print_path(parent, start_node, path_found_end_node, num_cols);
    } else {
        printf("Nenhum caminho encontrado por BFS.\n");
    }
    free_queue(q);
}

/**
 * @brief Função recursiva para Busca em Profundidade (DFS).
 *
 * @param graph O grafo.
 * @param current_node O nó atual sendo visitado.
 * @param end_node O nó de chegada.
 * @param visited Array para marcar nós visitados.
 * @param parent Array para reconstruir o caminho.
 * @param num_cols Número de colunas do labirinto (para print_path).
 * @return true se o nó de chegada foi encontrado a partir do current_node, false caso contrário.
 */
bool dfs_recursive(Graph* graph, int current_node, int end_node, bool visited[], int parent[], int num_cols) {
    visited[current_node] = true;

    // Se encontramos o nó de chegada, retornamos true
    if (current_node == end_node) {
        return true;
    }

    AdjListNode* temp = graph->adj_lists[current_node];
    while (temp) {
        int neighbor = temp->dest;
        if (!visited[neighbor]) {
            parent[neighbor] = current_node; // Define o pai do vizinho
            if (dfs_recursive(graph, neighbor, end_node, visited, parent, num_cols)) {
                return true; // Se o caminho foi encontrado por este vizinho, propaga o sucesso
            }
        }
        temp = temp->next;
    }
    return false; // Nenhum caminho encontrado a partir deste nó
}

/**
 * @brief Inicia a Busca em Profundidade (DFS).
 *
 * @param graph O grafo que representa o labirinto.
 * @param start_node O índice do nó de partida.
 * @param end_node O índice do nó de chegada.
 * @param num_rows Número de linhas do labirinto.
 * @param num_cols Número de colunas do labirinto.
 */
void dfs(Graph* graph, int start_node, int end_node, int num_rows, int num_cols) {
    printf("\n--- Iniciando Busca em Profundidade (DFS) ---\n");

    bool visited[graph->num_nodes];
    int parent[graph->num_nodes]; // Para reconstruir o caminho

    for (int i = 0; i < graph->num_nodes; i++) {
        visited[i] = false;
        parent[i] = -1;
    }

    if (dfs_recursive(graph, start_node, end_node, visited, parent, num_cols)) {
        printf("Caminho encontrado por DFS:\n");
        print_path(parent, start_node, end_node, num_cols);
    } else {
        printf("Nenhum caminho encontrado por DFS.\n");
    }
}


// --- Função Principal ---

int main() {
    // Exemplo de labirinto (pode ser ajustado)
    char maze[MAX_ROWS][MAX_COLS] = {
        {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#'},
        {'#', 'S', ' ', '#', ' ', ' ', ' ', '#', 'E', '#'},
        {'#', ' ', ' ', '#', ' ', '#', ' ', '#', ' ', '#'},
        {'#', ' ', '#', '#', ' ', '#', ' ', ' ', ' ', '#'},
        {'#', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', '#'},
        {'#', '#', '#', '#', '#', '#', ' ', '#', ' ', '#'},
        {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
        {'#', ' ', '#', '#', '#', '#', '#', '#', ' ', '#'},
        {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
        {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#'}
    };

    int num_rows = 10;
    int num_cols = 10;
    int num_nodes = num_rows * num_cols;

    Graph* graph = create_graph(num_nodes);

    int start_node = -1;
    int end_node = -1;

    // Direções: Cima, Baixo, Esquerda, Direita
    int dr[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};

    // Construir o grafo a partir do labirinto
    for (int r = 0; r < num_rows; r++) {
        for (int c = 0; c < num_cols; c++) {
            if (maze[r][c] == '#') {
                continue; // Paredes não são nós
            }

            int u = map_coord_to_index(r, c, num_cols);

            if (maze[r][c] == 'S') {
                start_node = u;
            } else if (maze[r][c] == 'E') {
                end_node = u;
            }

            // Adicionar arestas para vizinhos válidos
            for (int i = 0; i < 4; i++) {
                int nr = r + dr[i]; // Nova linha
                int nc = c + dc[i]; // Nova coluna

                if (is_valid(nr, nc, num_rows, num_cols) && maze[nr][nc] != '#') {
                    int v = map_coord_to_index(nr, nc, num_cols);
                    // Não adicionar a aresta duas vezes se já existir no grafo
                    // (A função add_edge já adiciona bidirecionalmente)
                    // Poderíamos verificar se já existe, mas para este caso simples,
                    // adicionar duas vezes não causa erro, apenas redundância de memória.
                    // Para um grafo mais complexo, seria necessário verificar.
                    add_edge(graph, u, v);
                }
            }
        }
    }

    if (start_node == -1 || end_node == -1) {
        printf("Erro: Ponto de partida 'S' ou de chegada 'E' não encontrado no labirinto.\n");
        free_graph(graph);
        return 1;
    }

    printf("Labirinto:\n");
    for (int r = 0; r < num_rows; r++) {
        for (int c = 0; c < num_cols; c++) {
            printf("%c ", maze[r][c]);
        }
        printf("\n");
    }

    // Executar BFS
    bfs(graph, start_node, end_node, num_rows, num_cols);

    // Executar DFS
    dfs(graph, start_node, end_node, num_rows, num_cols);

    // Liberar memória alocada para o grafo
    free_graph(graph);

    return 0;
}