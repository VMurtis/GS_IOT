# 🌦️ Projeto de Monitoramento Climático com ESP32

Este projeto utiliza um ESP32 para monitorar eventos climáticos extremos, como **ondas de calor**, **tempestades**, **ventos fortes** e **chuva intensa**, combinando sensores locais e dados da API do [OpenWeatherMap](https://openweathermap.org/). Os dados são exibidos em um display LCD e enviados para um broker MQTT, podendo ser visualizados no **Node-RED** ou outras plataformas IoT.

---

## 📦 Componentes Utilizados

- ESP32 Dev Module
- Sensor de temperatura e umidade **DHT22**
- Sensor de chuva (analógico)
- Display **LCD 16x2** com módulo **I2C**
- LEDs (indicadores de alerta): vermelho, amarelo, azul, verde
- Botão de modo
- Botão de teste de LEDs
- Conexão Wi-Fi
- MQTT (Mosquitto Broker)
- API OpenWeatherMap

---

## 📡 Funcionalidades

| Função                          | Descrição |
| ----------------------------- | --------- |
| 🌡️ Temperatura e umidade local | Captura via DHT22 |
| ☁️ Clima remoto via API        | Integração com OpenWeatherMap |
| ⚠️ Alertas climáticos          | LEDs e mensagens MQTT para:
  - Onda de calor  
  - Tempestade  
  - Ventos fortes  
  - Chuva intensa |
| 📟 LCD                         | Mostra dados de temperatura e umidade |
| 🧪 Teste de LEDs               | Botão de verificação visual dos LEDs de alerta |
| 🔁 Envio periódico             | Dados são enviados a cada 60 segundos |

---

## 🔧 Instalação

1. Clone este repositório:

   ```bash
   git clone https://github.com/seu-usuario/nome-do-repositorio.git

