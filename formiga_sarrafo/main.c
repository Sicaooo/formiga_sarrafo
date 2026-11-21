#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define QNT_FORMIGAS 3
#define DELIMITADOR_FIM_SARRAFO -1
#define CORTE_NAO_CABE NULL
#define SUCESSO 0

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
typedef struct NoLista{
	struct NoLista* prox;
	struct NoLista* ant;
	int dado;
} NoLista;

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

void push_sobras(Formiga* formigas, int sobra, int idx) {
	NoLista* novo_no = malloc(sizeof(*novo_no));
	if (!novo_no) exit(EXIT_FAILURE);

	if (!formigas[idx].ultima_sobra) {
		formigas[idx].ultima_sobra = novo_no;
		novo_no->ant = NULL;
	}
	else {
		formigas[idx].ultima_sobra->prox = novo_no;
		novo_no->ant = formigas[idx].ultima_sobra;
		formigas[idx].ultima_sobra = novo_no;
	}

	novo_no->prox = NULL;
	novo_no->dado = sobra;
}

void push_formiga(Formiga* formigas, int dado, int* tamanho_restante_sarrafo, int tamanho_sarrafo, int idx) {
	NoLista* novo_no = malloc(sizeof(*novo_no));
	if (!novo_no) exit(EXIT_FAILURE);

	if (!formigas[idx].ultima_formiga) {
		formigas[idx].ultima_formiga = novo_no;
		novo_no->ant = NULL;
		push_sobras(formigas, tamanho_sarrafo, idx);
	}
	else {
		formigas[idx].ultima_formiga->prox = novo_no;
		novo_no->ant = formigas[idx].ultima_formiga;
		formigas[idx].ultima_formiga = novo_no;
	}

	novo_no->prox = NULL;

	novo_no->dado = (dado > *tamanho_restante_sarrafo) ? DELIMITADOR_FIM_SARRAFO : dado;

	if (novo_no->dado == DELIMITADOR_FIM_SARRAFO) {
		*tamanho_restante_sarrafo = tamanho_sarrafo;
		push_formiga(formigas, dado, tamanho_restante_sarrafo, tamanho_sarrafo, idx);
		push_sobras(formigas, *tamanho_restante_sarrafo, idx);
		++formigas[idx].qnt_sarrafos_formiga;
		return;
	}

	*tamanho_restante_sarrafo -= dado;

	(formigas[idx].ultima_sobra)->dado = *tamanho_restante_sarrafo;

	if (!*tamanho_restante_sarrafo) {
		*tamanho_restante_sarrafo = tamanho_sarrafo;
		++formigas[idx].qnt_sarrafos_formiga;
	}
}

void init_formigas(Formiga* formigas, int qnt_formigas) {
	memset(formigas, 0, qnt_formigas * sizeof(*formigas));
	for (int i = 0; i < qnt_formigas; ++i) formigas[i].qnt_sarrafos_formiga = 1;
}

void print_sarrafos(Formiga* formigas, int qnt_formigas, int idx) {
	for (int i = 0; i < qnt_formigas; ++i) {
		printf("Formiga %d\n", i + 1);

		NoLista* no_formiga_atual = formigas[i].ultima_formiga;
		NoLista* no_sobra_atual = formigas[i].ultima_sobra;

		if (!no_formiga_atual || !no_sobra_atual) continue;

		while (no_formiga_atual->ant) no_formiga_atual = no_formiga_atual->ant;
		while (no_sobra_atual->ant) no_sobra_atual = no_sobra_atual->ant;

		for (int num_sarrafo = 1; num_sarrafo <= formigas[idx].qnt_sarrafos_formiga; ++num_sarrafo) {
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

void atualizar_feromonios(float* __restrict feromonios, float taxa_evaporacao, int qnt_cortes) {
	for (int i = 0; i < qnt_cortes * qnt_cortes; ++i) feromonios[i] -= feromonios[i] * taxa_evaporacao;
}

void probabilidade_transicao(float* chance_visitar, float* feromonios, const float peso_feromonio, const float* matriz_cortes, const float peso_heuristica, int qnt_cortes) {
	float soma = 0.0f;
	for (int i = 0; i < qnt_cortes; ++i) {
		soma += powf(matriz_cortes[i], peso_heuristica) * pow(feromonios[i], peso_feromonio);
	}

	for (int i = 0; i < qnt_cortes; ++i) {
		chance_visitar[i] = (pow(feromonios[i], peso_feromonio) * pow(matriz_cortes[i], peso_heuristica)) / soma;
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


int main(void) {
	/*teste das funções
	Formiga teste[QNT_FORMIGAS];
	init_formigas(&teste, QNT_FORMIGAS);
	int tamanho = 5;
	push_formiga(teste, 1, &tamanho, 5, 0);
	push_formiga(teste, 2, &tamanho, 5, 0);
	push_formiga(teste, 3, &tamanho, 5, 0);
	print_sarrafos(teste, QNT_FORMIGAS, 0);
	*/

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

	int qnt_cortes = 3;
	Corte cortes[3] = { { 3, 10 },{ 4, 20 },{ 5, 30 } };
	Formiga formigas[QNT_FORMIGAS];
	init_formigas(formigas, QNT_FORMIGAS);
	const float peso_heuristica = 0.5f;
	const float peso_feromonio = 0.5f;
	const float feromonio_por_formiga = 1.0f;
	const float taxa_evaporacao = 0.1f;

	int* matriz_cortes = malloc(qnt_cortes * qnt_cortes * sizeof(*matriz_cortes));
	if (!matriz_cortes) {
		puts("Problema na alocacao de memoria para a matriz de cortes");
		return 1;
	}

	init_matriz_cortes(matriz_cortes, qnt_cortes, cortes);

	float* feromonios = malloc(qnt_cortes * qnt_cortes * sizeof(*feromonios));
	if (!feromonios) {
		puts("Problema na alocacao de memoria para matriz de feromonios");
		free(matriz_cortes);
		return 1;
	}

	init_matriz_feromonios(feromonios, qnt_cortes);	



	free(cortes);

	return 0;
}