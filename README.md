# Desafio prático - Desenvolvedor back-end C++
O desafio consiste em trabalhar com documentos para serem assinados digitalmente.
## Cronograma
| Etapa | Descrição | Status | Melhorias |
| --------------------------------------------- | ----------------------------------- | ------------ | ------------------------------ |
| Etapa 0 - Montar ambiente para desenvolvimento | Criar ambiente de desenvolvimento. Foram sugeridas as ferramentas CONAN, e as bibliotecas POCO e OPENSSL. | Concluído | Não consegui utilizar o CONAN, optei por utilizar conteners para "Isolar os ambientes" e garantir replicação. |
| Etapa 1 - Obtenção do resumo criptográfico | A partir de um documento fornecido, obter o resumo criptográfico do documento com SHA512. | Concluído | Necessário criar Testes unitários. |
| Etapa 2 - Realizar uma assinatura digital | Criada Classe para Para fazer toda a parte de assinatura do documento. | Concluído | Necessário criar Testes unitários. |
| Etapa 3 - Verificar a assinatura gerada | Criada Classe para validar a assinatura, aqui não consegui obter o "encapContentInfo". | Incompleta | Necessário criar Testes unitários, necessário validar melhor o motivo de o "elemento" encapContentInfo estar sem valor. |
| Etapa 4 - API REST | Através da Biblioteca do POCO, criou-se um servidor HTTP, que responde os requests. | Concluído | Está com muitos dados de forma HARDCODE, necessário melhorar, ou ver uma estratégia para deixar isso de forma "melhor" para manutenção. |
| Etapa 5 - Relatório das Atividades | Documentação escrita, com a forma de fazer o build, estudo de caso e pontos de melhoria. | Concluída | Necessário colocar itens referente a *pipelines* e ambientes de desenvolvimento. |
| Etapa 6 (OPCIONAL) - Relatório das Atividades | | Não inicializado | |
## Build do projeto
Esse desafio foi feito pensando em Maquinas que tenham S.O. GNU/Linux, para isso clone o repositório, acesse a pasta do projeto e execute os seguintes comandos:
```
mkdir -p build
cd build
cmake ..
make
```
**OBS**.: Podem correr falhas durante o Build, por falta de bibliotecas, sejam (openSSL e/ou POCO), para isso seria necessário instala-las, ou via Gerenciador de repositório ou baixando e compilando as ferramentas.
### Usando Dockerfile e Dockercompose
Foram adicionados dois arquivos para podermos rodar a aplicação de forma "conterizada", que possibilita desacoplar do ambiente do usuário, garantindo que seja possível executar a aplicação. Para isso é necessário ter a ferramenta `docker`instalada na maquina. Para executar, esteja dentro na pasta do projeto e execute o comando:
```
docker-compose up -d
```
Após todo o processo do docker, o conteiner estará rodando em segundo plano. Pode-se executar os testes que estão descritos na próxima seção.
Outro passo interessante é ver as assinaturas geradas, pra isso é necessário fazer algumas requisições de documento e após basta executar o comando:
```
docker exec signServer ls -lah /app/docs/signature/
```
E a saída será algo como:
```
total 16K
drwxr-xr-x 2 root root 4.0K May 18 18:12 .
drwxr-xr-x 4 root root 4.0K May 18 18:11 ..
-rw-r--r-- 1 root root  256 May 18 18:11 73317d474b4e062caf306c5652d00de0c64fed7547dc56f5062ed2336c02bc944b7b5775ceddaf0767c0b6afab2034f4f382e0ee3cf23ba73bd311ba370fe628.p7s
-rw-r--r-- 1 root root  256 May 18 18:12 df2501a803d62e8c4f54541a95ee2ca64e6a31b0fd88e3daab720dae2912a406f7f691879e37a7638ac908a2ab4a1f41bc99c53754f9c30dbf995451c94e991a.p7s
```
## Testando a implementação
Para testar a aplicação é necessário:
- Documento a ser assinado;
- Documento com informações do signatário;
- Senha do documento com as informações do signatário;
- Ter a aplicação compilada (ou acesso ao docker em execução);
A seguir um exemplo utilizando `terminal` (com `bash`) e com as ferramentas `sed`,  `curl`,  `base64` e `jq`.
Para gerar o documento assinado:
```
curl -F "document=@resources/arquivos/doc.txt" -F "pkcs12=@resources/pkcs12/certificado_teste_hub.pfx" -F "password=bry123456" http://localhost:8080/signature/ | jq .cms | sed 's/["]//g' | base64 -d > docRet.cms
```
Para validar a assinatura:
```
curl -F "cms=@docRet.cms" http://localhost:8080/verify/ | jq
```
Saída:
```
{
  "CN": "HUB2 TESTES",
  "digestAlgorithm": "sha256",
  "encapContentInfo": "",
  "signingTime": "250518140535Z",
  "status": "VALIDO"
}
```
## Estudo do Desafio
Foram enviados dois arquivos juntamente com o desafio. Um arquivo `.txt` e um arquivo `pfx` (que acaba sendo o formato `pkcs12`).
Com esses dois arquivos, e com a dica dada, relacionada a biblioteca `openssl`, foi feito um estudo do que se espera e como fazer isso utilizando o aplicativo `openssl` no terminal.
Informações sobre o sistema:
```
uname -a
```
Saída: `Linux silva 6.11.0-25-generic #25~24.04.1-Ubuntu SMP PREEMPT_DYNAMIC Tue Apr 15 17:20:50 UTC 2 x86_64 x86_64 x86_64 GNU/Linux`
Versão do **openssl**:
```
openssl version
```
Saída: `OpenSSL 3.0.13 30 Jan 2024 (Library: OpenSSL 3.0.13 30 Jan 2024)`
O primeiro passo, foi manipular o documento `.txt`, para ver o resumo em *SHA-512*, para confrontar com a saída da aplicação quando fosse implementada:
```
openssl sha512 doc.txt
```
Saída:`SHA2-512(doc.txt)= dc1a7de77c59a29f366a4b154b03ad7d99013e36e08beb50d976358bea7b045884fe72111b27cf7d6302916b2691ac7696c1637e1ab44584d8d6613825149e35`
Após isso, partiu-se para o documento `pkcs12`, onde atraves da senha fornecida, separou-se em dois arquivos. Sendo o primeiro a chave privada:
```
openssl pkcs12 -info -in certificado_teste_hub.pfx -nodes -nocerts -out certificado_teste_hub.key
```
E o segundo certificado:
```
openssl pkcs12 -info -in certificado_teste_hub.pfx -nokeys -out certificado_teste_hub.crt
```
Com esses dois artefatos, foi possível gerar o documento que será assinado no padrão *Cryptographic Message Syntax* (CMS):
```
openssl cms -sign -in doc.txt -out doc.msg -inkey certificado_teste_hub.key -signer certificado_teste_hub.crt 
```
Após isso Foi necessário gerar a assinatura, para isso, criou-se um os certificados RSA:
Privado:
```
openssl genpkey -algorithm RSA -out chave_privada.pem -pkeyopt rsa_keygen_bits:2048
```
Publico:
```
openssl rsa -in chave_privada.pem -pubout -out chave_publica.pem
```
Com os certificados, bastou apenas assinar o documento com *SHA-512*:
```
openssl dgst -sha512 -sign chave_privada.pem -out assinatura.bin doc.msg
```
Com o documento assinado, foi feita a verificação da autenticidade do documento:
```
openssl dgst -sha512 -verify chave_publica.pem -signature assinatura.bin doc.msg
```
Saída: `Verified OK`
Além da autenticidade verificada, explorou-se algumas informações do documento assinado CMS:
```
openssl cms -cmsout -print -in doc.msg
```
A partir desse ponto, bastou replicar o que foi feito aqui em **C++** que pode ser visto dentro do projeto.
## Implementação
Para a implementação, usou-se a documentação das bibliotecas [openSSL]( https://docs.openssl.org/) e [POCO](https://pocoproject.org/). Além de buscas por exemplos na internet e [videos](https://www.youtube.com/watch?v=O1OaJmrRHrw&list=PLgBMtP0_D_afzNG7Zs2jr8FSoyeU4yqhi).
### Dificuldades e melhorias
- Questões referentes a implementação, como não tenho experiencia com *C++* tentou-se aproximar de algo estruturado, poderia explorar melhor questões que o paradigma de *POO*.
- Existem muitas coisas em **HARDCODE**, que poderia-se buscar estratégias melhores para isso, ou criar arquivos de configuração.
- **Não** foi configurado o gerenciador *CONAN*, logo não se tem garantia de versão de bibliotecas, portanto ficou-se diretamente ligado a "linkagem" com as bibliotecas do SO.
- **Não** consegui manipular todas as informações do CMS, como o *encapContentInfo*, confesso que isso foi bem frustante, como não sei se gerei corretamente o arquivo, ou se deveria fazer um *SHA-512* do arquivo ou ainda faltou algum detalhe durante a assinatura. Fato é que não consegui extrair a informação como devia.
- Tive muita dificuldade em trabalhar com a *API* da biblioteca do *openSSL*, muitos exemplos que eu encontrei utilizavam coisas que eram de versões antigas, e a documentação é um tanto quanto difícil de entender a utilização dos métodos. 
- Será necessário repensar uma maneira de gerenciar as assinaturas, uma vez que, pelo que eu entendi cada assinatura é exclusiva para cada documento, no modelo que eu fiz, se a assinatura não existe, nem é impresso os outros dados do *CMS*, seria necessário rever essa questão para ver se é interessante, mesmo que o documento não seja valido, imprimir as informações.