#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// ============================================================
// PINOS DA CAIXA
// ============================================================
constexpr uint8_t UART_RX_PIN = 16;
constexpr uint8_t UART_TX_PIN = 17;
constexpr uint8_t BUTTON_PIN = 4;
constexpr uint8_t LED_STRIP_PIN = 23;
constexpr uint8_t READ_PIN = 18;

// Ajuste para a quantidade real de LEDs da fita.
constexpr uint16_t LED_COUNT = 8;
constexpr uint8_t LED_BRIGHTNESS = 40;

// ============================================================
// CONFIGURACOES DAS COMUNICACOES
// ============================================================
constexpr uint32_t UART_BAUD_RATE = 115200;
constexpr uint32_t BIT_TIME_US = 1000;
constexpr uint32_t LINE_TIMEOUT_MS = 10000;
constexpr uint32_t FRAME_TIMEOUT_MS = 7000;
constexpr uint32_t BYTE_TIMEOUT_MS = 100;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 30;

constexpr uint8_t SOF = 0xA5;
constexpr uint8_t MAX_IDS = 16;

Adafruit_NeoPixel ledStrip(
  LED_COUNT,
  LED_STRIP_PIN,
  NEO_GRB + NEO_KHZ800
);

struct ProtocolPacket
{
  uint8_t count;
  uint8_t ids[MAX_IDS];
};

enum class BoxStatus : uint8_t
{
  WAITING,
  RESETTING,
  READING,
  SUCCESS,
  ERROR
};

void setStatusLed(BoxStatus status)
{
  uint32_t color = 0;

  switch (status)
  {
    case BoxStatus::WAITING:
      color = ledStrip.Color(0, 0, 255);       // Azul
      break;

    case BoxStatus::RESETTING:
      color = ledStrip.Color(255, 100, 0);     // Amarelo/laranja
      break;

    case BoxStatus::READING:
      color = ledStrip.Color(0, 255, 255);     // Ciano
      break;

    case BoxStatus::SUCCESS:
      color = ledStrip.Color(0, 255, 0);       // Verde
      break;

    case BoxStatus::ERROR:
      color = ledStrip.Color(255, 0, 0);       // Vermelho
      break;
  }

  for (uint16_t i = 0; i < LED_COUNT; i++)
  {
    ledStrip.setPixelColor(i, color);
  }

  ledStrip.show();
}

bool waitIdleHigh(uint32_t timeoutMs)
{
  const uint32_t start = millis();

  while (digitalRead(READ_PIN) == LOW)
  {
    if ((millis() - start) >= timeoutMs)
    {
      return false;
    }
  }

  // Confirma que a linha permaneceu em repouso HIGH.
  delay(50);
  return digitalRead(READ_PIN) == HIGH;
}

bool receiveByte(uint8_t &data, uint32_t timeoutMs)
{
  const uint32_t start = millis();
  uint8_t received = 0;

  // Aguarda o start bit em LOW.
  while (digitalRead(READ_PIN) == HIGH)
  {
    if ((millis() - start) >= timeoutMs)
    {
      return false;
    }
  }

  // Vai para o centro do start bit.
  delayMicroseconds(BIT_TIME_US / 2);

  if (digitalRead(READ_PIN) != LOW)
  {
    return false;
  }

  // Vai para o centro do primeiro bit de dados.
  delayMicroseconds(BIT_TIME_US);

  // Recebe 8 bits, do menos significativo para o mais significativo.
  for (uint8_t bit = 0; bit < 8; bit++)
  {
    if (digitalRead(READ_PIN) == HIGH)
    {
      received |= static_cast<uint8_t>(1U << bit);
    }

    delayMicroseconds(BIT_TIME_US);
  }

  // Confere o stop bit.
  if (digitalRead(READ_PIN) != HIGH)
  {
    return false;
  }

  data = received;
  return true;
}

uint8_t checksum(const ProtocolPacket &packet)
{
  uint8_t value = packet.count;

  for (uint8_t i = 0; i < packet.count; i++)
  {
    value ^= packet.ids[i];
  }

  return value;
}

bool receivePacket(ProtocolPacket &packet)
{
  const uint32_t frameStart = millis();
  uint8_t value = 0;

  // Procura o byte inicial 0xA5.
  do
  {
    const uint32_t elapsed = millis() - frameStart;

    if (elapsed >= FRAME_TIMEOUT_MS)
    {
      return false;
    }

    if (!receiveByte(value, FRAME_TIMEOUT_MS - elapsed))
    {
      return false;
    }
  }
  while (value != SOF);

  if (!receiveByte(packet.count, BYTE_TIMEOUT_MS))
  {
    return false;
  }

  if ((packet.count == 0) || (packet.count > MAX_IDS))
  {
    return false;
  }

  for (uint8_t i = 0; i < packet.count; i++)
  {
    if (!receiveByte(packet.ids[i], BYTE_TIMEOUT_MS))
    {
      return false;
    }
  }

  if (!receiveByte(value, BYTE_TIMEOUT_MS))
  {
    return false;
  }

  return value == checksum(packet);
}

void sendPacketByUart(const ProtocolPacket &packet)
{
  Serial2.write(SOF);
  Serial2.write(packet.count);
  Serial2.write(packet.ids, packet.count);
  Serial2.write(checksum(packet));
  Serial2.flush();
}

void printPacket(const ProtocolPacket &packet)
{
  Serial.print("Quantidade de blocos: ");
  Serial.println(packet.count);

  Serial.print("IDs recebidos: [");

  for (uint8_t i = 0; i < packet.count; i++)
  {
    if (i > 0)
    {
      Serial.print(", ");
    }

    Serial.print(packet.ids[i]);
  }

  Serial.println("]");
}

void readBlocks()
{
  Serial.println();
  Serial.println("Botao solto. Aguardando a inicializacao dos blocos...");
  setStatusLed(BoxStatus::READING);

  if (!waitIdleHigh(LINE_TIMEOUT_MS))
  {
    Serial.println("Erro: o sinal nao entrou em repouso HIGH.");
    setStatusLed(BoxStatus::ERROR);
    return;
  }

  Serial.println("Linha em repouso. Aguardando o pacote...");

  ProtocolPacket packet{};

  if (!receivePacket(packet))
  {
    Serial.println("Erro: pacote ausente, incompleto ou invalido.");
    setStatusLed(BoxStatus::ERROR);
    return;
  }

  printPacket(packet);
  sendPacketByUart(packet);

  Serial.println("Pacote encaminhado pela UART2.");
  setStatusLed(BoxStatus::SUCCESS);
}

void handleButton()
{
  static uint8_t lastReading = HIGH;
  static uint8_t stableState = HIGH;
  static uint32_t lastChangeMs = 0;
  static bool resetWasRequested = false;

  const uint8_t reading = digitalRead(BUTTON_PIN);

  if (reading != lastReading)
  {
    lastReading = reading;
    lastChangeMs = millis();
  }

  if ((millis() - lastChangeMs) < BUTTON_DEBOUNCE_MS)
  {
    return;
  }

  if (reading == stableState)
  {
    return;
  }

  stableState = reading;

  if (stableState == LOW)
  {
    // O botao pressionado desliga/reseta os blocos pelo circuito externo.
    resetWasRequested = true;
    Serial.println("Botao pressionado: blocos em reset.");
    setStatusLed(BoxStatus::RESETTING);
    return;
  }

  // A leitura comeca somente depois de um pressionamento confirmado
  // e quando o botao volta para HIGH, energizando novamente os blocos.
  if (resetWasRequested)
  {
    resetWasRequested = false;
    readBlocks();
  }
}

void setup()
{
  Serial.begin(115200);

  Serial2.begin(
    UART_BAUD_RATE,
    SERIAL_8N1,
    UART_RX_PIN,
    UART_TX_PIN
  );

  pinMode(READ_PIN, INPUT_PULLDOWN);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  ledStrip.begin();
  ledStrip.setBrightness(LED_BRIGHTNESS);
  ledStrip.clear();
  ledStrip.show();

  Serial.println();
  Serial.println("Caixa inicializada.");
  Serial.println("Pressione e solte o botao para resetar e ler os blocos.");

  setStatusLed(BoxStatus::WAITING);
}

void loop()
{
  handleButton();
}
