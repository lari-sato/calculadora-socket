# Projeto – Computação Distribuída

## Dupla
- Julia Santos Oliveira – RA 10417672 
- Larissa Yuri Sato – RA 10418318 

---

## Como compilar e executar

### Compilação com `make`
Na raiz do projeto:
```bash
make          # compila servidor e cliente
make server   # compila apenas o servidor
make client   # compila apenas o cliente
make clean    # remove binários
```

### Execução
Suba o servidor (porta padrão 5050):
```bash
./server 5050
```

Abra outro terminal e conecte o cliente (mesmo procedimento para múltiplos clientes):
```bash
./client 127.0.0.1 5050
```

---

## Formato do protocolo

- **Prefixo**  
  ```
  ADD A B
  SUB A B
  MUL A B
  DIV A B
  ```
- **Infixo**  
  ```
  A + B
  A - B
  A * B
  A / B
  ```

- **Comandos de controle**
  - `HELP` → mostra instruções do protocolo  
  - `VERSION` → versão atual do servidor  
  - `QUIT` → encerra a sessão do cliente  

### Respostas
- Sucesso:
  ```
  OK <resultado>
  ```
  Exemplo:
  ```
  OK 12.000000
  ```
- Erro:
  ```
  ERR <CODIGO> <mensagem>
  ```
  Exemplos:
  ```
  ERR EZDV divisao_por_zero
  ERR EINV entrada_invalida
  ```

---

## Decisões de projeto e limitações
- O servidor usa **TCP IPv4** e multiplexação com `select()`, aceitando múltiplos clientes concorrentes. 
  - Implementar o suporte a **múltiplos clientes** mantendo buffers por conexão e garantindo que cada solicitação receba a resposta correta exigiu cuidado com montagem de linhas e estados.
- A rotina que avalia expressões trata **prefixo e infixo** na mesma função, pois ao tentar separar em duas, a estrutura se mostrou praticamente idêntica e mudou apenas de onde vinha o operador.  
- Saída numérica fixa com `printf("%.6f")` e locale `LC_NUMERIC="C"` para garantir **ponto** como separador decimal.  
- Tratamento explícito de erros: entradas inválidas, divisão por zero.  
- Encerramento de cliente via `QUIT`, e encerramento do servidor via `Ctrl+C`.  
- Não chegamos a implementar um **arquivo de testes automatizados**, pois o processo se msotrou complexo para nós. Não sabíamos como fazer com que ele interagisse com o cliente para comparar as saídas, e não tínhamos conhecimento suficiente sobre scripts (TSV).

---

## Como testar!
- **Prefixo**  
  ```
  ADD 10 2
  SUB 7 9 
  MUL -3 3.5
  DIV 45 9 
  DIV 10 0
  ```
- **Infixo**  
  ```
  10 + 2
  7 - 9
  -3 * 3.5
  45 / 9
  10 / 0
  ```
- **Inválido**  
  ```
  FOO 8 9
  ADD 1 
  10 + 2x
  45 / 9
  ```
- **Comandos**  
  ```
  HELP
  VERSION
  QUIT
  ```