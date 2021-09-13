typedef struct message{
	unsigned long int id;
	unsigned long int fd;
	int type;
}Message;

typedef struct lista {
    Message *info;
    struct lista* prox;
}Lista;

typedef struct fila {
    Lista* ini;
    Lista* fim;
}Fila;

Fila* fila_cria();

void fila_insere(Fila *f,Message *v);

int fila_vazia(Fila *f);

Message* fila_retira(Fila *f);

void fila_libera(Fila *f);

