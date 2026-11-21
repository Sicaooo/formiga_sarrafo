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
dado: armazena o valor naquele no
*/
typedef struct NoLista{
	struct NoLista* prox;
	int dado;
} NoLista;

/*
Estrutura das formigas. Cada posição dos vetores representa uma formiga
inicio_formiga: um vetor contendo o endereço do primeiro resultado da formiga 
ultima_formiga: um vetor contendo o endereço do ultimo resultado da formiga
inicio_sobra: um vetor contendo o endereço do primeiro resultado de sobra
ultima_sobra: um vetor contendo o endereço do ultimo resultado de sobra
qnt_sarrafos_formiga: um vetor contendo a quantidade de sarrafos utilizados
Exemplo: Se eu quiser acessar o tour da segunda formiga eu faria um laço
percorrendo cada um dos nós começando em Formigas->inicio_formiga[1]
*/
typedef struct {
	NoLista** inicio_formiga;
	NoLista** ultima_formiga;
	NoLista** inicio_sobra;
	NoLista** ultima_sobra;
	int* qnt_sarrafos_formiga;
} Formigas;

void push_sobras(Formigas* formigas, int sobra, int idx) {
	NoLista* novo_no = malloc(sizeof(*novo_no));
	if (!novo_no) exit(EXIT_FAILURE);

	if (!formigas->ultima_sobra[idx]) formigas->ultima_sobra[idx] = formigas->inicio_sobra[idx] = novo_no;
	else {
		formigas->ultima_sobra[idx]->prox = novo_no;
		formigas->ultima_sobra[idx] = novo_no;
	}

	novo_no->prox = NULL;
	novo_no->dado = sobra;
}

void push_formiga(Formigas* formigas, int dado, int* tamanho_restante_sarrafo, int tamanho_sarrafo, int idx) {
	NoLista* novo_no = malloc(sizeof(*novo_no));
	if (!novo_no) exit(EXIT_FAILURE);

	if (!formigas->ultima_formiga[idx]) {
		formigas->ultima_formiga[idx] = formigas->inicio_formiga[idx] = novo_no;
		push_sobras(formigas, tamanho_sarrafo, idx);
	}
	else {
		formigas->ultima_formiga[idx]->prox = novo_no;
		formigas->ultima_formiga[idx] = novo_no;
	}

	novo_no->prox = NULL;

	novo_no->dado = (dado > *tamanho_restante_sarrafo) ? DELIMITADOR_FIM_SARRAFO : dado;

	if (novo_no->dado == DELIMITADOR_FIM_SARRAFO) {
		*tamanho_restante_sarrafo = tamanho_sarrafo;
		push_formiga(formigas, dado, tamanho_restante_sarrafo, tamanho_sarrafo, idx);
		push_sobras(formigas, *tamanho_restante_sarrafo, idx);
		++formigas->qnt_sarrafos_formiga[idx];
		return;
	}

	*tamanho_restante_sarrafo -= dado;

	(*formigas->ultima_sobra)->dado = *tamanho_restante_sarrafo;

	if (!*tamanho_restante_sarrafo) {
		*tamanho_restante_sarrafo = tamanho_sarrafo;
		++formigas->qnt_sarrafos_formiga[idx];
	}
}

void init_formigas(Formigas* formigas, int qnt_formigas) {
	formigas->inicio_formiga = malloc(sizeof(*formigas->inicio_formiga) * qnt_formigas);
	formigas->ultima_formiga = malloc(sizeof(*formigas->inicio_formiga) * qnt_formigas);
	formigas->inicio_sobra = malloc(sizeof(*formigas->inicio_formiga) * qnt_formigas);
	formigas->ultima_sobra = malloc(sizeof(*formigas->inicio_formiga) * qnt_formigas);
	formigas->qnt_sarrafos_formiga = malloc(sizeof(*formigas->qnt_sarrafos_formiga));

	if (!formigas->inicio_formiga || !formigas->ultima_formiga || !formigas->inicio_sobra || !formigas->ultima_sobra || !formigas->qnt_sarrafos_formiga) {
		free(formigas->inicio_formiga);
		free(formigas->ultima_formiga);
		free(formigas->inicio_sobra);
		free(formigas->ultima_sobra);
		free(formigas->qnt_sarrafos_formiga);

		exit(EXIT_FAILURE);
	}

	memset(formigas->inicio_formiga, 0, qnt_formigas * sizeof(*formigas->inicio_formiga));
	memset(formigas->ultima_formiga, 0, qnt_formigas * sizeof(*formigas->ultima_formiga));
	memset(formigas->inicio_sobra, 0, qnt_formigas * sizeof(*formigas->inicio_sobra));
	memset(formigas->ultima_sobra, 0, qnt_formigas * sizeof(*formigas->ultima_sobra));
	for (int i = 0; i < qnt_formigas; ++i) formigas->qnt_sarrafos_formiga[i] = 1;
}

void print_sarrafos(Formigas* formigas, int qnt_formigas, int idx) {
	for (int i = 0; i < qnt_formigas; ++i) {
		printf("Formiga %d\n", i + 1);

		NoLista* no_formiga_atual = formigas->inicio_formiga[i];
		NoLista* no_sobra_atual = formigas->inicio_sobra[i];

		for (int num_sarrafo = 1; num_sarrafo <= formigas->qnt_sarrafos_formiga[idx]; ++num_sarrafo) {
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
	Formigas teste;
	init_formigas(&teste, 3);
	int tamanho = 5;
	push_formiga(&teste, 1, &tamanho, 5, 0);
	push_formiga(&teste, 2, &tamanho, 5, 0);
	push_formiga(&teste, 3, &tamanho, 5, 0);
	print_sarrafos(&teste, 1, 0);

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