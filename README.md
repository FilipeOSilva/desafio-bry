# Desafio prático - Desenvolvedor back-end C++

O desafio consiste em trabalhar com documentos para serem assinados digitalmente.
## Cronograma
| Etapa                                         | Descrição                           | Status       | Melhorias                      |
| --------------------------------------------- | ----------------------------------- | ------------ | ------------------------------ |
| Etapa 0 - Montar ambiente para desenvolvimento | Criar ambiente de desenvolvimento. Foram sugeridas as ferramentas CONAN, e as bibliotecas POCO e OPENSSL. | Concluído | Não consegui utilizar o CONAN, optei por utilizar conteners para "Isolar os ambientes" e garantir replicação. |
| Etapa 1 - Obtenção do resumo criptográfico | A partir de um documento fornecido, obter o resumo criptográfico do documento com SHA512. | Concluído | Necessário criar Testes unitários. |
| Etapa 2 - Realizar uma assinatura digital | Criada Classe para Para fazer toda a parte de assinatura do documento. | Concluído | Necessário criar Testes unitários. |
| Etapa 3 - Verificar a assinatura gerada | Criada Classe para validar a assinatura, aqui não consegui obter o "encapContentInfo". | Incompleta   | Necessário criar Testes unitários, necessário validar melhor o motivo de o "elemento" encapContentInfo estar sem valor. |
| Etapa 4 - API REST | Através da Biblioteca do POCO, criou-se um servidor HTTP, que responde os requests. | Concluído | Está com muitos dados de forma HARDCODE, necessário melhorar, ou ver uma estratégia para deixar isso de forma "melhor" para manutenção. |
| Etapa 5 - Relatório das Atividades | Esse Readme, onde está pendente a documentação para Rodar a APP | Pendente |  |
| Etapa 6 (OPCIONAL) - Relatório das Atividades | | Não inicializado | |

## Build do projeto
```
mkdir -p build 
cd build 
cmake .. 
make
```