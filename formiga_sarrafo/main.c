#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DELIMITADOR_FIM_SARRAFO -1
#define CORTE_NAO_CABE NULL
#define SUCESSO 0

typedef struct {
	int* quantidade;
	int* tamanho;
} Cortes;

typedef struct NoLista {
	struct NoLista* prox;
	int dado;
} NoLista;

// Agora cada struct representa UMA formiga
typedef struct {
	NoLista* inicio_formiga;
	NoLista* ultima_formiga;
	NoLista* inicio_sobra;
	NoLista* ultima_sobra;
	int qnt_sarrafos_formiga;
} Formiga;

// Não precisa mais do parâmetro 'idx'! Recebe direto a formiga desejada
void push_sobras(Formiga* formiga, int sobra) {
	NoLista* novo_no = malloc(sizeof(*novo_no));
	if (!novo_no) exit(EXIT_FAILURE);

	if (!formiga->ultima_sobra) {
		formiga->ultima_sobra = formiga->inicio_sobra = novo_no;
	}
	else {
		formiga->ultima_sobra->prox = novo_no;
		formiga->ultima_sobra = novo_no;
	}

	novo_no->prox = NULL;
	novo_no->dado = sobra;
}

void push_formiga(Formiga* formiga, int dado, int* tamanho_restante_sarrafo, int tamanho_sarrafo) {
	NoLista* novo_no = malloc(sizeof(*novo_no));
	if (!novo_no) exit(EXIT_FAILURE);

	if (!formiga->ultima_formiga) {
		formiga->ultima_formiga = formiga->inicio_formiga = novo_no;
	}
	else {
		formiga->ultima_formiga->prox = novo_no;
		formiga->ultima_formiga = novo_no;
	}

	novo_no->prox = NULL;
	novo_no->dado = (dado > *tamanho_restante_sarrafo) ? DELIMITADOR_FIM_SARRAFO : dado;

	if (novo_no->dado == DELIMITADOR_FIM_SARRAFO) {
		push_sobras(formiga, *tamanho_restante_sarrafo);
		return;
	}

	*tamanho_restante_sarrafo -= dado;

	if (!*tamanho_restante_sarrafo) {
		push_sobras(formiga, 0);
		*tamanho_restante_sarrafo = tamanho_sarrafo;
		++formiga->qnt_sarrafos_formiga;
	}
}

int pop_lista(NoLista** inicio, NoLista** ultimo, int* valor) {
	if (*inicio == NULL) {
		return 0;
	}

	NoLista* temp = *inicio;
	*valor = temp->dado;
	*inicio = (*inicio)->prox;

	if (*inicio == NULL) {
		*ultimo = NULL;
	}

	free(temp);
	return 1;
}

// Muito mais simples: zera apenas uma formiga por vez
void init_formiga(Formiga* formiga) {
	formiga->inicio_formiga = NULL;
	formiga->ultima_formiga = NULL;
	formiga->inicio_sobra = NULL;
	formiga->ultima_sobra = NULL;
	formiga->qnt_sarrafos_formiga = 1;
}

// Passamos o 'idx' aqui só para exibir o número bonito no printf
void imprimir_formiga(Formiga* f, int idx) {
	printf("=== Formiga %d (Sarrafos: %d) ===\n", idx, f->qnt_sarrafos_formiga);

	printf("  Cortes: ");
	for (NoLista* aux = f->inicio_formiga; aux != NULL; aux = aux->prox) {
		if (aux->dado == DELIMITADOR_FIM_SARRAFO) {
			printf("| ");
		}
		else {
			printf("%d ", aux->dado);
		}
	}
	printf("\n");

	printf("  Sobras: ");
	for (NoLista* aux = f->inicio_sobra; aux != NULL; aux = aux->prox) {
		printf("%d ", aux->dado);
	}
	printf("\n\n");
}

void construir_tour_corte(Formiga* formigas, int idx_formiga, Cortes* cortes, int qnt_tipos_cortes, int tam_sarrafo, double** feromonio) {
	// 1. Copia a quantidade de cortes que precisam ser feitos
	int* qtd_restante = malloc(sizeof(int) * qnt_tipos_cortes);
	memcpy(qtd_restante, cortes->quantidade, sizeof(int) * qnt_tipos_cortes);

	// Calcula o total de peças que a formiga precisa cortar no total
	int total_pecas = 0;
	for (int i = 0; i < qnt_tipos_cortes; i++) {
		total_pecas += qtd_restante[i];
	}

	int espaco_restante = tam_sarrafo;
	int corte_anterior = 0; // Começa olhando a linha 0 da matriz de feromônio (ou sorteia o 1º)

	// 2. Enquanto ainda faltarem peças para cortar:
	for (int passo = 0; passo < total_pecas; passo++) {
		double soma = 0.0;
		double* prob = calloc(qnt_tipos_cortes, sizeof(double));
		int algum_cabe = 0;

		// Verifica quais cortes têm estoque E cabem no sarrafo atual
		for (int j = 0; j < qnt_tipos_cortes; j++) {
			if (qtd_restante[j] > 0 && cortes->tamanho[j] <= espaco_restante) {
				algum_cabe = 1;
				int sobra = espaco_restante - cortes->tamanho[j];
				double heuristica = 1.0 / (sobra + 1.0); // Substitui o (1 / distancia)
				soma += heuristica * feromonio[corte_anterior][j];
			}
		}

		// Se NENHUM corte disponível cabe no espaço que sobrou, fecha o sarrafo e abre um novo!
		if (!algum_cabe) {
			// Força o fechamento chamando push ou resetando manualmente
			inserir_no_fim(&formigas->inicio_formiga[idx_formiga], &formigas->ultima_formiga[idx_formiga], DELIMITADOR_FIM_SARRAFO);
			push_sobras(formigas, espaco_restante, idx_formiga);
			espaco_restante = tam_sarrafo;
			formigas->qnt_sarrafos_formiga[idx_formiga]++;
			passo--; // Não gastou uma peça nesse passo, então repete a rodada com sarrafo cheio
			free(prob);
			continue;
		}

		// Calcula as probabilidades (igualzinho ao seu código das lojas!)
		for (int e = 0; e < qnt_tipos_cortes; e++) {
			if (qtd_restante[e] > 0 && cortes->tamanho[e] <= espaco_restante) {
				int sobra = espaco_restante - cortes->tamanho[e];
				double heuristica = 1.0 / (sobra + 1.0);
				prob[e] = (heuristica * feromonio[corte_anterior][e]) / soma;
			}
		}

		// Chama a sua função roleta para sortear o índice 'proximo'
		int proximo = roleta(prob, qnt_tipos_cortes);

		// Executa o corte na formiga e dá baixa no estoque dela
		push_formiga(formigas, cortes->tamanho[proximo], &espaco_restante, tam_sarrafo, idx_formiga);
		qtd_restante[proximo]--;
		corte_anterior = proximo;

		free(prob);
	}

	free(qtd_restante);
}



int main(void) {
	int qtd = 3; // ou scanf("%d", &qtd);

	// Aloca o vetor de 'qtd' formigas
	Formiga* teste = malloc(sizeof(Formiga) * qtd);
	if (!teste) return 1;

	// Inicializa cada formiga no seu loop
	for (int i = 0; i < qtd; i++) {
		init_formiga(&teste[i]);
	}

	int tamanho = 5;
	// Agora você passa o endereço da formiga específica: &teste
	for (int i = 0; i < qtd; i++) {

		int tamanho = 5;
		int numero = (rand() % 4) + 1;
		push_formiga(&teste[i], numero, &tamanho, 5);
		numero = (rand() % 4) + 1;
		push_formiga(&teste[i], numero, &tamanho, 5);
		numero = (rand() % 4) + 1;
		push_formiga(&teste[i], numero, &tamanho, 5);
	}

	for (int i = 0; i < qtd; i++) {
		imprimir_formiga(&teste[i], i);
	}

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


	/*

	for(geração)










	*/






	free(cortes.tamanho);
	free(cortes.quantidade);





	// Não esqueça de liberar as listas internas e depois o vetor 'teste'
	free(teste);

	return 0;
}


int xd() {


	Formigas teste;
	init_formigas(&teste, 3);
	int tamanho = 5;
	push_formiga(&teste, 1, &tamanho, 5, 0);
	push_formiga(&teste, 2, &tamanho, 5, 0);
	push_formiga(&teste, 3, &tamanho, 5, 0);


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
}
