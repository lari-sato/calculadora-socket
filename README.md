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
  ```bash
  HELP      # mostra instruções do protocolo  
  VERSION   # versão atual do servidor  
  QUIT      #encerra a sessão do cliente
  ```

### Respostas
- Sucesso:
  ```
  OK <resultado>
  ```
- Erro:
  ```
  ERR <CODIGO> <mensagem>
  ```
  - Exemplos:
    ```
    ERR EZDV divisao_por_zero
    ERR EINV entrada_invalida
    ```

---

## Decisões de projeto e Limitações 

### Decisões

#### Requisitos Mínimos e Recomendados
- **Servidor TCP (IPv4)** funcional na porta indicada, aceitando múltiplos clientes.
- **Cliente de terminal** lê linhas do `stdin`, envia ao servidor e imprime a resposta de forma síncrona.  
- **Parsing robusto**, validando quantidade de tokens, tipos numéricos e operação. 
- **Tratamento de erros**: entradas inválidas resultam em `ERR EINV entrada_invalida`; divisão por zero resulta em `ERR EZDV divisao_por_zero`.  
- **Resultados formatados** com `printf("%.6f")` e `LC_NUMERIC="C"`, evitando dependência do locale e garantindo ponto decimal.  
- **Makefile** feito com targets `all`, `server`, `client`, `clean`, simplificando a compilação.   
- **Logs no Servidor** que registram conexões, requisições e erros.  
- **Encerramento do Servidor** ao receber `SIGINT` (`Ctrl+C`).  
- **Parametrização do Cliente** com endereço e porta -> (`./client 127.0.0.1 5050`).  
- **Suporte a números negativos** usando `strtod()`, para conversão numérica.  
- **Mensagens Padronizadas** do servidor em código  (`OK`/`ERR`), como demonstrado.

#### Bônus
- **Concorrência**: Implementada via `select()`. Foi trabalhoso estruturar buffers independentes para cada conexão e garantir que todas as mensagens fossem montadas e respondidas corretamente.  
- **Testes automatizados**: Não foram implementados. O processo se mostrou complexo, pois não sabíamos como integrar os scripts de teste com o cliente para validar automaticamente as saídas.  
- **Protocolo estendido**: 
    - Forma Infixa: Implementada na mesma rotina que avalia expressões prefixas, pois ao tentar separar as duas, a estrutura se mostrou praticamente idêntica, e mudou apenas de onde vinha o operador. 
    - Mensagens `HELP`e `VERSION`implementadas.

### Limitações 
- **Parsing restrito**: apenas operações binárias simples são aceitas; não há suporte a expressões complexas com parênteses ou múltiplos operadores.  
- **Testes automatizados**: não foram implementados; a validação é feita manualmente.  
- **Formato estrito para entradas negativas**: números negativos precisam seguir o formato esperado (sem espaço entre o sinal e o número, como em `-3 * 2`). Não seria reconhecido: `- 3`.

---

## Como testar!
### Exemplos de Entrada:

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
  ```bash
  FOO 8 9   # operação desconhecida
  ADD 1     # tokens insuficientes
  10 + 2x   # lixo após número
  ```
- **Comandos**  
  ```
  HELP
  VERSION
  QUIT
  ```