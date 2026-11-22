#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define QNT_FORMIGAS 3
#define DELIMITADOR_FIM_SARRAFO -1
//Constante para deposito de feromonio: Q
#define Q 1
//rô "p" taxa de evaporação: p
#define p 0.1f
#define SUCESSO 0
#define CORTE_NAO_EXISTE 1

typedef struct {
	int quantidade;
	int tamanho; //(cm)
} Corte;

/*
No das listas usadas pela estrutura de Formigas
prox: ponteiro para o próximo elemento da lista
ant: ponteiro para o elemento anterior da lista
dado: armazena o valor naquele no
*/
typedef struct NoLista {
	struct NoLista* prox;
	struct NoLista* ant;
	int dado;
} NoLista;

/*
Estrutura para os sarrafos de tamanhos variaveis
*/
// typedef struct {
// 	NoLista* ultimo_sarrafo;
// 	int qnt_sarrafos;
// } Sarrafos;


/*
Estrutura de cada uma das formigas
ultima_formiga: ponteiro para o último resultado da formiga
ultima_sobra: ponteiro para o ultimo resultado de sobra da formiga
qnt_sarrafos_formiga: quantos sarrafos a formiga usou
*/
typedef struct {
	NoLista* ultima_formiga;
	NoLista* ultima_sobra;
	int qnt_sarrafos_formiga;
} Formiga;

void push_sobras(Formiga* formigas, int sobra) {
	NoLista* novo_no = malloc(sizeof(*novo_no));
	if (!novo_no) exit(EXIT_FAILURE);

	if (!formigas->ultima_sobra) {
		formigas->ultima_sobra = novo_no;
		novo_no->ant = NULL;
	}
	else {
		formigas->ultima_sobra->prox = novo_no;
		novo_no->ant = formigas->ultima_sobra;
		formigas->ultima_sobra = novo_no;
	}

	novo_no->prox = NULL;
	novo_no->dado = sobra;
}

// Função auxiliar: converte o tamanho do corte para o índice dele no vetor 'cortes'
int buscar_indice_corte(Corte* cortes, int qnt_cortes, int tamanho_corte) {
	if (tamanho_corte == DELIMITADOR_FIM_SARRAFO) return 1;
	for (int i = 0; i < qnt_cortes; i++) {
		if (cortes[i].tamanho == tamanho_corte) {
			return i;
		}
	}
	return -1;
}

int push_formiga(Corte* cortes, int qnt_cortes, Formiga* formigas, int dado, int* tamanho_restante_sarrafo, int tamanho_sarrafo) {
	int pos = buscar_indice_corte(cortes, qnt_cortes, dado);
	if (pos == -1) return CORTE_NAO_EXISTE;

	NoLista* novo_no = malloc(sizeof(*novo_no));
	if (!novo_no) exit(EXIT_FAILURE);

	if (!formigas->ultima_formiga) {
		formigas->ultima_formiga = novo_no;
		novo_no->ant = NULL;
		push_sobras(formigas, tamanho_sarrafo);
	}
	else {
		formigas->ultima_formiga->prox = novo_no;
		novo_no->ant = formigas->ultima_formiga;
		formigas->ultima_formiga = novo_no;
	}

	novo_no->prox = NULL;

	novo_no->dado = (dado > *tamanho_restante_sarrafo) ? DELIMITADOR_FIM_SARRAFO : dado;

	if (novo_no->dado == DELIMITADOR_FIM_SARRAFO) {
		*tamanho_restante_sarrafo = tamanho_sarrafo;
		push_formiga(cortes, qnt_cortes, formigas, dado, tamanho_restante_sarrafo, tamanho_sarrafo);
		push_sobras(formigas, *tamanho_restante_sarrafo);
		++formigas->qnt_sarrafos_formiga;
		return SUCESSO;
	}

	*tamanho_restante_sarrafo -= dado;

	(formigas->ultima_sobra)->dado = *tamanho_restante_sarrafo;

	--cortes[pos].quantidade;

	if (!*tamanho_restante_sarrafo) {
		*tamanho_restante_sarrafo = tamanho_sarrafo;
		++formigas->qnt_sarrafos_formiga;
	}

	return SUCESSO;
}

void init_formigas(Corte* cortes, int qnt_cortes, Formiga* formigas, int qnt_formigas, int* sarrafo_restante, int tamanho_sarrafo) {
	memset(formigas, 0, qnt_formigas * sizeof(*formigas));
	for (int i = 0; i < qnt_formigas; ++i) {
		formigas[i].qnt_sarrafos_formiga = 1;

		push_formiga(cortes + (i * qnt_cortes), qnt_cortes, formigas + i, cortes[i].tamanho, sarrafo_restante + i, tamanho_sarrafo);
	}
	
}

void print_sarrafos(Formiga* formigas, int qnt_formigas) {
	for (int i = 0; i < qnt_formigas; ++i) {
		printf("Formiga %d\n", i + 1);

		NoLista* no_formiga_atual = formigas[i].ultima_formiga;
		NoLista* no_sobra_atual = formigas[i].ultima_sobra;

		if (!no_formiga_atual || !no_sobra_atual) continue;

		while (no_formiga_atual->ant) no_formiga_atual = no_formiga_atual->ant;
		while (no_sobra_atual->ant) no_sobra_atual = no_sobra_atual->ant;

		for (int num_sarrafo = 1; num_sarrafo <= formigas[i].qnt_sarrafos_formiga; ++num_sarrafo) {
			if (num_sarrafo != 1) printf("\n");
			printf("Cortes sarrafo %d: ", num_sarrafo);
			while (no_formiga_atual) {
				if (no_formiga_atual->dado == DELIMITADOR_FIM_SARRAFO) {
					no_formiga_atual = no_formiga_atual->prox;
					break;
				}
				printf("%d ", no_formiga_atual->dado);
				no_formiga_atual = no_formiga_atual->prox;
			}

			printf("\nSobra sarrafo %d: %d\n", num_sarrafo, no_sobra_atual->dado);
		}

		printf("\n");
	}
}

void init_matriz_cortes(int* matriz_cortes, int qnt_cortes, Corte* cortes) {
	for (int i = 0; i < qnt_cortes; ++i) {
		for (int j = 0; j < qnt_cortes; ++j) {
			matriz_cortes[j + (i * qnt_cortes)] = cortes[j].tamanho;
		}
	}
}

void init_matriz_feromonios(float* feromonios, int qnt_cortes) {
	for (int i = 0; i < qnt_cortes * qnt_cortes; ++i) {
		feromonios[i] = 1.0f;
	}
}

double funcao_objetivo(Formiga* formiga, int idx, int tamanho_sarrafo) {
	double aproveitamento = 0.0f;
	int qnt_sarrafos = formiga[idx].qnt_sarrafos_formiga;
	NoLista* aux_sobra_sarrafos = formiga[idx].ultima_sobra;

	if (qnt_sarrafos <= 0 || aux_sobra_sarrafos == NULL) return 0.0;

	for (int i = 0; i < qnt_sarrafos && aux_sobra_sarrafos != NULL; i++) {
		aproveitamento += pow(1.0 - ((double)aux_sobra_sarrafos->dado / tamanho_sarrafo), 2);
		aux_sobra_sarrafos = aux_sobra_sarrafos->ant;
	}

	return aproveitamento / qnt_sarrafos;
}

// Atualização Depósito proporcional direto (Q * F_k)
void atualizar_feromonios(float* feromonios, Formiga* formigas, Corte* cortes, int qnt_cortes, int tamanho_sarrafo) {
	//evaporação (1 - rô) * tau_ij(t) em toda a matriz
	int total_elementos = qnt_cortes * qnt_cortes;
	for (int i = 0; i < total_elementos; i++) {
		feromonios[i] *= (1.0f - p);
	}

	//deposito somatório de (Q * F_k) para cada formiga k
	for (int k = 0; k < QNT_FORMIGAS; k++) {
		double fitness = funcao_objetivo(formigas, k, tamanho_sarrafo);
		float delta = Q * (float)fitness;

		NoLista* atual = formigas[k].ultima_formiga;

		while (atual != NULL && atual->ant != NULL) {
			int corte_j = atual->dado;
			int corte_i = atual->ant->dado;

			// Ignora transições que cruzam a divisão entre sarrafos (-1)
			if (corte_i != DELIMITADOR_FIM_SARRAFO && corte_j != DELIMITADOR_FIM_SARRAFO) {
				int idx_i = buscar_indice_corte(cortes, qnt_cortes, corte_i);
				int idx_j = buscar_indice_corte(cortes, qnt_cortes, corte_j);

				if (idx_i != -1 && idx_j != -1) {
					feromonios[idx_j + (idx_i * qnt_cortes)] += delta;
				}
			}

			atual = atual->ant;
		}
	}
}

void probabilidade_transicao(Corte* cortes, float* chance_visitar, float* feromonios, const float peso_feromonio, const int* matriz_cortes, int qnt_cortes, const float peso_heuristica) {
	float soma = 0.0f;
	for (int i = 0; i < qnt_cortes; ++i) {
		soma += powf(matriz_cortes[i], peso_heuristica) * powf(feromonios[i], peso_feromonio) * (cortes[i].quantidade > 0);
	}

	for (int i = 0; i < qnt_cortes; ++i) {
		chance_visitar[i] = (cortes[i].quantidade) ? (powf(feromonios[i], peso_feromonio) * powf(matriz_cortes[i], peso_heuristica)) / soma : 0.0f;
	}
}

int escolha_transicao(float* chance_visitar, int qnt_cortes) {
	float num_sorteio = (float)(rand() % 10000) / 10000.0f;

	float chances_acumuladas = 0.0f;
	int i;
	for (i = 0; i < qnt_cortes; ++i) {
		chances_acumuladas += chance_visitar[i];
		if (chances_acumuladas && chances_acumuladas >= num_sorteio) break;
	}

	return i;
}

void gerar_numeros_aleatorios(void) {
	srand(time(NULL));
	int limit = rand() % 1000;
	for (int i = 0; i < limit; ++i) rand();
}

void inicializar_posicoes(int* caminhos, int qnt_cortes) {
	for (int i = 0; i < qnt_cortes; ++i) {
		caminhos[i * qnt_cortes] = i;
	}
}

int main(void) {
	//int tam_sarrafos;
	//printf("Insira o tamanho dos sarrafos (cm): ");
	//scanf("%d", &tam_sarrafos);

	//Corte* cortes;

	//int qnt_cortes;
	//printf("Insira a quantidade de cortes: ");
	//scanf("%d", &qnt_cortes);

	//cortes = malloc(qnt_cortes * sizeof(*cortes));
	//if (!cortes) {
	//	puts("Problema na alocacao de memoria para armazenar os cortes");
	//	return 1;
	//}

	//puts("Insira o tamanho do corte (cm) e a quantidade separado por um espaco");
	//for (int i = 0; i < qnt_cortes; ++i) {
	//	scanf("%d %d", &cortes[i].tamanho, &cortes[i].quantidade);
	//}
	gerar_numeros_aleatorios();

	int qnt_cortes = 3;
	Corte cortes[3][3] = { 
		{ { 3, 10 },{ 4, 20 },{ 5, 30 } },
		{ { 3, 10 },{ 4, 20 },{ 5, 30 } },
		{ { 3, 10 },{ 4, 20 },{ 5, 30 } }
	};
	
	Formiga formigas[QNT_FORMIGAS];
	
	const int TAMANHO_SARRAFO = 100;
	
	int sarrafo_restante[3];
	for (int i = 0; i < qnt_cortes; ++i) sarrafo_restante[i] = TAMANHO_SARRAFO;

	init_formigas(*cortes, qnt_cortes, formigas, qnt_cortes, sarrafo_restante, TAMANHO_SARRAFO);
	
	NoLista* debug_formiga = formigas->ultima_formiga;
	NoLista* debug_sobras = formigas->ultima_sobra;
	
	const float peso_heuristica = 0.5f;
	const float peso_feromonio = 0.5f;

	int* matriz_cortes = malloc(qnt_cortes * qnt_cortes * sizeof(*matriz_cortes));
	if (!matriz_cortes) {
		puts("Problema na alocacao de memoria para a matriz de cortes");
		return 1;
	}

	init_matriz_cortes(matriz_cortes, qnt_cortes, *cortes);

	float* feromonios = malloc(qnt_cortes * qnt_cortes * sizeof(*feromonios));
	if (!feromonios) {
		puts("Problema na alocacao de memoria para matriz de feromonios");
		free(matriz_cortes);
		return 1;
	}

	init_matriz_feromonios(feromonios, qnt_cortes);

	float* chance_visitar = malloc(qnt_cortes * sizeof(*chance_visitar));
	if (!chance_visitar) return 1;

	const int MAX_GERACOES = 1000;
	for (int geracao = 0; geracao < MAX_GERACOES; ++geracao) {
		*sarrafo_restante = TAMANHO_SARRAFO;
		for (int i = 0; i < qnt_cortes; ++i) {
			for (int j = 1; j < qnt_cortes; ++j) {
				probabilidade_transicao(*cortes + (i * qnt_cortes), chance_visitar, feromonios + (j - 1) * qnt_cortes, peso_feromonio, matriz_cortes + (j - 1), qnt_cortes, peso_heuristica);
				int proximo_corte = escolha_transicao(chance_visitar, qnt_cortes);
				push_formiga(*cortes + (i * qnt_cortes), qnt_cortes, formigas + i, cortes[i][proximo_corte].tamanho, &sarrafo_restante, TAMANHO_SARRAFO);
			}
		}

		print_sarrafos(formigas, qnt_cortes);
	}

	return 0;
}