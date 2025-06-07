#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h> // Para INT_MAX
#include <string.h>

// --- Definições Globais e Estruturas ---

#define MAX_NODES 20 // Número máximo de paradas/estações na rede
#define INFINITY INT_MAX // Representa uma distância infinita (não conectada)

// Estrutura para um nó na lista de adjacência (representa uma aresta)
typedef struct AdjListNode {
    int dest; // Índice do nó de destino
    int weight; // Peso da aresta (tempo de deslocamento)
    struct AdjListNode* next;
} AdjListNode;

// Estrutura para o Grafo (Lista de Adjacência)
typedef struct Graph {
    int num_nodes;
    AdjListNode** adj_lists; // Array de ponteiros para listas de adjacência
    char** node_names;       // Nomes das estações/paradas
} Graph;

// --- Funções Auxiliares do Grafo ---

// Cria um novo nó da lista de adjacência
AdjListNode* create_adj_list_node(int dest, int weight) {
    AdjListNode* new_node = (AdjListNode*)malloc(sizeof(AdjListNode));
    if (!new_node) {
        perror("Erro ao alocar AdjListNode");
        exit(EXIT_FAILURE);
    }
    new_node->dest = dest;
    new_node->weight = weight;
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
    graph->node_names = (char**)malloc(num_nodes * sizeof(char*));

    if (!graph->adj_lists || !graph->node_names) {
        perror("Erro ao alocar listas de adjacência ou nomes dos nós");
        free(graph->adj_lists);
        free(graph->node_names);
        free(graph);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_nodes; i++) {
        graph->adj_lists[i] = NULL;
        graph->node_names[i] = NULL; // Inicializa com NULL, será preenchido depois
    }
    return graph;
}

// Adiciona uma aresta direcionada ao grafo (de src para dest com peso)
void add_edge(Graph* graph, int src, int dest, int weight) {
    // Adiciona dest à lista de src
    AdjListNode* new_node = create_adj_list_node(dest, weight);
    new_node->next = graph->adj_lists[src];
    graph->adj_lists[src] = new_node;
}

// Define o nome de um nó
void set_node_name(Graph* graph, int node_index, const char* name) {
    if (node_index < 0 || node_index >= graph->num_nodes) {
        fprintf(stderr, "Erro: Índice de nó inválido.\n");
        return;
    }
    // Aloca espaço para o nome e copia
    graph->node_names[node_index] = (char*)malloc(strlen(name) + 1);
    if (!graph->node_names[node_index]) {
        perror("Erro ao alocar nome do nó");
        exit(EXIT_FAILURE);
    }
    strcpy(graph->node_names[node_index], name);
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
        free(graph->node_names[i]); // Libera o nome do nó
    }
    free(graph->adj_lists);
    free(graph->node_names);
    free(graph);
}

// --- Algoritmo de Dijkstra ---

/**
 * @brief Implementa o algoritmo de Dijkstra para encontrar o caminho de menor custo
 * de um nó de origem para todos os outros nós.
 *
 * @param graph O grafo de transporte.
 * @param start_node O índice do nó de partida.
 * @param dist Array para armazenar as distâncias mínimas do nó de partida.
 * @param parent Array para armazenar os predecessores para reconstrução do caminho.
 */
void dijkstra(Graph* graph, int start_node, int dist[], int parent[]) {
    bool visited[graph->num_nodes];

    // Inicializa distâncias como INFINITY e visitados como false
    for (int i = 0; i < graph->num_nodes; i++) {
        dist[i] = INFINITY;
        visited[i] = false;
        parent[i] = -1; // -1 indica nenhum pai
    }

    dist[start_node] = 0; // Distância do nó inicial para ele mesmo é 0

    // Encontra o caminho mais curto para todos os vértices
    for (int count = 0; count < graph->num_nodes - 1; count++) {
        // Encontra o vértice com a menor distância não visitada
        int min_dist = INFINITY;
        int u = -1;

        for (int v = 0; v < graph->num_nodes; v++) {
            if (!visited[v] && dist[v] <= min_dist) {
                min_dist = dist[v];
                u = v;
            }
        }

        if (u == -1) break; // Todos os nós alcançáveis foram processados

        visited[u] = true; // Marca o nó como visitado

        // Atualiza as distâncias dos vértices adjacentes ao nó 'u'
        AdjListNode* current = graph->adj_lists[u];
        while (current) {
            int v = current->dest;
            int weight = current->weight;

            // Se 'v' não foi visitado e existe um caminho mais curto através de 'u'
            if (!visited[v] && dist[u] != INFINITY && dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                parent[v] = u; // Define 'u' como pai de 'v'
            }
            current = current->next;
        }
    }
}

// --- Funções de Impressão e Interação ---

// Imprime o caminho encontrado do início ao fim
void print_path(Graph* graph, int parent[], int start_node, int end_node) {
    if (end_node == start_node) {
        printf("Você já está em '%s'.\n", graph->node_names[start_node]);
        return;
    }
    if (parent[end_node] == -1 && end_node != start_node) {
        printf("Não há caminho disponível de '%s' para '%s'.\n",
               graph->node_names[start_node], graph->node_names[end_node]);
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

    printf("Melhor trajeto:\n");
    for (int i = path_len - 1; i >= 0; i--) {
        printf("-> %s", graph->node_names[path[i]]);
    }
    printf("\n");
}

// --- Função Principal ---

int main() {
    // Nomes das estações/paradas
    const char* station_names[] = {
        "Centro", "Rodoviaria", "Shopping", "Parque", "Hospital",
        "Aeroporto", "Praia", "Bairro Norte", "Bairro Sul", "Terminal Central"
    };
    int num_stations = 10; // Usando apenas as 10 primeiras para este exemplo

    Graph* graph = create_graph(num_stations);

    // Atribui os nomes aos nós do grafo
    for (int i = 0; i < num_stations; i++) {
        set_node_name(graph, i, station_names[i]);
    }

    // Define as arestas (conexões e tempos de deslocamento)
    // Formato: add_edge(grafo, origem_idx, destino_idx, tempo_em_minutos);
    add_edge(graph, 0, 1, 10); // Centro -> Rodoviaria (10 min)
    add_edge(graph, 0, 2, 15); // Centro -> Shopping (15 min)
    add_edge(graph, 1, 0, 12); // Rodoviaria -> Centro (12 min - pode ser diferente!)
    add_edge(graph, 1, 3, 20); // Rodoviaria -> Parque (20 min)
    add_edge(graph, 2, 4, 8);  // Shopping -> Hospital (8 min)
    add_edge(graph, 3, 5, 25); // Parque -> Aeroporto (25 min)
    add_edge(graph, 4, 1, 7);  // Hospital -> Rodoviaria (7 min)
    add_edge(graph, 4, 6, 18); // Hospital -> Praia (18 min)
    add_edge(graph, 5, 9, 30); // Aeroporto -> Terminal Central (30 min)
    add_edge(graph, 6, 9, 22); // Praia -> Terminal Central (22 min)
    add_edge(graph, 7, 0, 5);  // Bairro Norte -> Centro (5 min)
    add_edge(graph, 8, 0, 8);  // Bairro Sul -> Centro (8 min)
    add_edge(graph, 9, 5, 28); // Terminal Central -> Aeroporto (28 min)
    add_edge(graph, 9, 6, 20); // Terminal Central -> Praia (20 min)
    add_edge(graph, 3, 8, 10); // Parque -> Bairro Sul (10 min)


    printf("Bem-vindo ao Sistema de Rotas de Transporte Público!\n");
    printf("Estações disponíveis:\n");
    for (int i = 0; i < num_stations; i++) {
        printf("%2d. %s\n", i, graph->node_names[i]);
    }

    int start_index = -1;
    int end_index = -1;

    // Entrada interativa do usuário
    printf("\nSelecione o ponto de partida (digite o número): ");
    scanf("%d", &start_index);
    if (start_index < 0 || start_index >= num_stations) {
        printf("Índice de partida inválido.\n");
        free_graph(graph);
        return 1;
    }

    printf("Selecione o ponto de destino (digite o número): ");
    scanf("%d", &end_index);
    if (end_index < 0 || end_index >= num_stations) {
        printf("Índice de destino inválido.\n");
        free_graph(graph);
        return 1;
    }

    printf("\nCalculando rota de '%s' para '%s'...\n",
           graph->node_names[start_index], graph->node_names[end_index]);

    int dist[MAX_NODES];   // Distância mínima do início para cada nó
    int parent[MAX_NODES]; // Predecessor no caminho mais curto

    dijkstra(graph, start_index, dist, parent);

    printf("\n--- Resultado do Trajeto ---\n");
    printf("Tempo mínimo de viagem de '%s' para '%s': %d minutos.\n",
           graph->node_names[start_index], graph->node_names[end_index],
           (dist[end_index] == INFINITY) ? -1 : dist[end_index]); // -1 se não houver caminho

    if (dist[end_index] != INFINITY) {
        print_path(graph, parent, start_index, end_index);
    }

    // Liberar memória alocada para o grafo
    free_graph(graph);

    return 0;
}