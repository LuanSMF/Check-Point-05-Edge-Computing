# O Caso da Vinheria Agnello — IoT e FIWARE

Integrantes do grupo

- **João Nóbrega** — RM 570322

- **Kevin Simões** — RM 571942

- **Luan Sá** — RM 569136

- **Flávia Camerim** — RM 570979

## 1. Descrição do projeto

Este projeto implementa um sistema de monitoramento ambiental para uma adega da Vinheria Agnello, utilizando IoT, MQTT e a plataforma FIWARE.

O sistema coleta dados de sensores simulados no ESP32 por meio do Wokwi, transmite as informações via MQTT, exibe as medições em um dashboard e armazena o histórico para consulta.

## 2. Objetivos

- Monitorar temperatura, umidade relativa e luminosidade.
- Transmitir dados por MQTT.
- Integrar os dados à plataforma FIWARE.
- Visualizar as medições em tempo real por meio do Node-RED.
- Consultar dados históricos com o STH-Comet.
- Permitir o acionamento de LEDs por comandos MQTT.

## 3. Tecnologias utilizadas

- **Wokwi e ESP32:** simulação do dispositivo IoT.
- **DHT22:** medição de temperatura e umidade relativa.
- **LDR:** leitura analógica de luminosidade.
- **MQTT:** comunicação entre os componentes.
- **Mosquitto:** broker MQTT local.
- **FIWARE Orion Context Broker:** gerenciamento do contexto dos dispositivos.
- **FIWARE IoT Agent UltraLight:** integração das mensagens MQTT ao Orion.
- **STH-Comet:** consulta ao histórico das medições.
- **Node-RED:** processamento e visualização dos dados.
- **Docker Compose:** execução dos serviços em containers.

## 4. Arquivos do projeto

- `sketch.ino`: código do ESP32.
- `diagram.json`: montagem dos componentes no Wokwi.
- `fluxo-node-red.json`: fluxo exportado do Node-RED.
- `docker-compose.yml`: configuração dos serviços Docker.
- `README.md`: documentação e instruções de reprodução.

## 5. Arquitetura do sistema

O ESP32 simulado coleta as medições e publica mensagens no broker MQTT público. O Node-RED recebe essas mensagens, processa os valores e atualiza o dashboard. Os dados também são encaminhados ao broker local e integrados ao FIWARE pelo IoT Agent. O Orion gerencia as entidades de contexto, enquanto o STH-Comet permite consultar o histórico.

Os comandos MQTT enviados pelo aplicativo MyMQTT permitem controlar os LEDs conectados ao ESP32.

## 6. Configuração e execução

### 6.1. Iniciar os serviços Docker

No Ubuntu, acesse a pasta que contém o arquivo `docker-compose.yml`:

```bash
cd ~/fiware
```

Inicie os serviços:

```bash
sudo docker compose up -d
```

Confira o estado dos containers:

```bash
sudo docker compose ps
```

Para consultar os registros dos serviços:

```bash
sudo docker compose logs --tail=100
```

### 6.2. Acessar os serviços

Os endereços abaixo pressupõem que as portas estejam publicadas e acessíveis na máquina em que o navegador está sendo executado.

| Serviço | Endereço |
|---|---|
| Node-RED | `http://localhost:1880` |
| Orion Context Broker | `http://localhost:1026` |
| IoT Agent UltraLight | `http://localhost:4041` |
| STH-Comet | `http://localhost:8666` |

Se o Docker estiver em uma máquina virtual, substitua `localhost` pelo IP do Ubuntu quando acessar os serviços a partir do Windows.

### 6.3. Importar o fluxo do Node-RED

1. Abra o Node-RED.
2. Acesse o menu principal.
3. Selecione a opção de importar fluxo.
4. Selecione `fluxo-node-red.json`.
5. Confira as configurações dos nós MQTT e dos serviços FIWARE.
6. Clique em **Deploy**.

Verifique os endereços dos brokers, as credenciais e os tópicos antes de executar o fluxo.

### 6.4. Executar a simulação Wokwi

1. Abra o projeto no Wokwi.
2. Confira a montagem dos sensores no `diagram.json`.
3. Execute a simulação do ESP32.
4. Verifique se as mensagens MQTT estão sendo publicadas.

O dispositivo utiliza a rede `Wokwi-GUEST` e publica dados no tópico:

`/TEF/agnelloiris/attrs`

O broker público utilizado é `test.mosquitto.org`, na porta `1883`.

### 6.5. Visualizar os dados

O dashboard do Node-RED apresenta indicadores de temperatura, umidade e luminosidade.

Endereço do dashboard utilizado durante o desenvolvimento:

`http://localhost:1880/dashboard/page2`

O valor do LDR é uma leitura bruta do sensor, não uma medição calibrada em lux.

## 7. Comunicação MQTT

### Tópicos utilizados

| Tópico | Finalidade |
|---|---|
| `/TEF/agnelloiris/attrs` | Publicação dos atributos dos sensores |
| `/TEF/agnelloiris/cmd` | Recebimento dos comandos de controle |
| `/ul/TEF/agnelloiris/attrs` | Encaminhamento dos dados ao broker local |

A mensagem de telemetria utiliza o formato UltraLight, com identificadores para temperatura (`t`), umidade (`h`), luminosidade (`l`) e instante da medição (`TimeInstant`).

Exemplo ilustrativo:

```text
t|25.6|h|40.0|l|24|TimeInstant|2026-10-23T22:28:09Z
```

Os valores acima são apenas um exemplo de formato; as medições reais variam durante a simulação.

### Comandos de controle

Os comandos enviados ao tópico `/TEF/agnelloiris/cmd` são:

- `agnelloiris@on`: aciona os LEDs configurados.
- `agnelloiris@off`: desliga os LEDs configurados.

O comportamento depende da montagem e do código presentes no projeto Wokwi.

## 8. Integração FIWARE

O dispositivo integrado pelo IoT Agent utiliza a entidade:

`urn:ngsi-ld:VinheriaAgnello:agnelloiris`

Tipo:

`VinheriaAgnello`

O serviço utiliza o identificador de serviço `openiot`, a chave de API `TEF` e o caminho de serviço `/`.

Também foi utilizada uma entidade de demonstração baseada no Smart Data Model `IndoorEnvironmentObserved`, com identificador:

`urn:ngsi-ld:IndoorEnvironmentObserved:adega-demo`

Essa entidade organiza informações ambientais como temperatura, umidade relativa, instante da observação e localização.

A umidade relativa é representada como uma fração: por exemplo, `0.4` corresponde a 40%.

A localização demonstrativa não deve ser interpretada como a localização real da adega.

## 9. Histórico das medições

O STH-Comet é utilizado para consultar o histórico dos atributos associados às entidades integradas ao Orion.

Durante os testes, foram verificadas consultas históricas de temperatura, umidade e luminosidade da entidade original, além de temperatura, umidade relativa e data de observação na entidade ambiental de demonstração.

A disponibilidade do histórico depende do funcionamento das assinaturas, da configuração do STH-Comet e do recebimento contínuo de atualizações.

## 10. Validação do sistema

Os testes realizados durante o desenvolvimento incluíram:

- Recebimento das mensagens dos sensores via MQTT.
- Atualização dos indicadores do dashboard.
- Integração dos atributos com o Orion Context Broker.
- Consulta de dados históricos no STH-Comet.
- Leitura dos tópicos e envio de comandos pelo MyMQTT.
- Acionamento dos LEDs conforme os comandos recebidos.

## 11. Limitações conhecidas

- Os sensores e o ESP32 são simulados no Wokwi.
- A leitura do LDR não foi calibrada para fornecer valores físicos em lux.
- O broker MQTT público é compartilhado e não deve ser tratado como ambiente seguro de produção.
- Os endereços e as portas dependem da configuração de rede da máquina virtual.
- Para reproduzir o ambiente em outra máquina, pode ser necessário ajustar credenciais, endereços de brokers, assinaturas e configurações dos nós.

## 12. Conclusão

O projeto demonstra a integração de sensores simulados, comunicação MQTT, processamento com Node-RED, gerenciamento de contexto com FIWARE e consulta histórica de dados.

A solução reúne os principais componentes de uma arquitetura IoT e permite acompanhar condições ambientais, visualizar as medições e testar comandos de controle remotamente.
