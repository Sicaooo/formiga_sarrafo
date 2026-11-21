#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DELIMITADOR_FIM_SARRAFO -1
#define CORTE_NAO_CABE NULL
#define SUCESSO 0

/*
Armazena as informações dos cortes
tamanho: vetor de tamanho dos cortes
quantidade: vetor da quantidade de cada corte
Exemplo: as informações do primeiro corte serão acessadas por
Cortes->quantidade[0]; e Cortes->tamanho[0];
*/
typedef struct {
	int* quantidade;
	int* tamanho;
} Cortes;

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

int main(void) {
	//teste das funções
	Formiga teste[3];
	init_formigas(&teste, 3);
	int tamanho = 5;
	push_formiga(teste, 1, &tamanho, 5, 0);
	push_formiga(teste, 2, &tamanho, 5, 0);
	push_formiga(teste, 3, &tamanho, 5, 0);
	print_sarrafos(teste, 1, 0);

	int tam_sarrafos;
	printf("Insira o tamanho dos sarrafos (cm): ");
	scanf("%d", &tam_sarrafos);

	Cortes cortes;

	int qnt_cortes;
	printf("Insira a quantidade de cortes: ");
	scanf("%d", &qnt_cortes);

	cortes.quantidade = malloc(qnt_cortes * sizeof(*cortes.quantidade));
	cortes.tamanho = malloc(qnt_cortes * sizeof(*cortes.tamanho));
	if (!cortes.quantidade || !cortes.tamanho) {
		puts("problema na alocacao de memoria para os cortes");

		free(cortes.tamanho);
		free(cortes.quantidade);

		return 1;
	}

	puts("Insira o tamanho do corte (cm) e a quantidade separado por um espaco");
	for (int i = 0; i < qnt_cortes; ++i) {
		scanf("%d %d", cortes.tamanho + i, cortes.quantidade + i);
	}

	free(cortes.tamanho);
	free(cortes.quantidade);

	return 0;
}