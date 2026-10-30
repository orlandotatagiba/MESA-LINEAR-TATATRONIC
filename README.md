# Controle de Mesa Linear - TATATRONIC

Sistema de controle de movimento linear (ciclo vai-e-vem automático e Jog manual) desenvolvido para **Arduino Uno**, utilizando driver de motor de passo **Leadshine DM555**, display **LCD 16x2 I2C** e **Teclado Matricial 4x3**.

---

## 🛠️ Hardware e Mapeamento de Pinos

| Componente | Pino no Arduino Uno | Observação |
| :--- | :--- | :--- |
| **Driver PUL+ (Step)** | Pino `9` | Obrigatório para a biblioteca `FastAccelStepper` |
| **Driver DIR+ (Direção)** | Pino `10` | Lógica invertida via software (`false`) |
| **Driver PUL- / DIR-** | `GND` | Negativo comum (chaveando positivo) |
| **Teclado Linhas (R1 a R4)** | Pinos `2, 3, 4, 5` | Teclado matricial 4x3 |
| **Teclado Colunas (C1 a C3)**| Pinos `6, 7, 8` | Teclado matricial 4x3 |
| **LCD 16x2 I2C (SDA / SCL)** | `A4` (SDA) / `A5` (SCL) | Endereço I2C: `0x27` |

### Parâmetros Mecânicos e de Movimento
* **Relação de Passos (`stepsPerMm`):** `1142.857` passos/mm
* **Velocidade Inicial:** `2500 Hz` (ajustável de `500 Hz` a `6000 Hz`)
* **Aceleração:** `2500 passos/s²`

---

## 🎮 Como Funciona (Operação pelo Teclado)

### 1. Definir a Distância do Curso (`0` a `9`)
* Com a mesa parada, dê **cliques rápidos** nas teclas numéricas (`0` a `9`) para digitar o tamanho do movimento desejado em milímetros (`Dist:` no display).

### 2. Iniciar Movimento Vai-e-Vem Contínuo (`#`)
* Após digitar uma distância maior que zero, pressione `#`.
* A mesa salvará a posição atual como ponto inicial e ficará indo e voltando continuamente na distância configurada até que a operação seja cancelada.

### 3. Parada Imediata / Cancelar (`*`)
* Pressione `*` a qualquer momento para **parar o motor imediatamente** (sem rampa de desaceleração) na posição exata em que estiver e zerar a distância digitada na tela.

### 4. Movimento Manual - JOG (`4` e `6`)
* Com o ciclo automático desligado, **mantenha pressionada por mais de 2 segundos**:
  * **Tecla `4` (Esquerda):** Move a mesa continuamente no sentido positivo (`<< ESQUERDA (4)`).
  * **Tecla `6` (Direita):** Move a mesa continuamente no sentido negativo (`DIREITA (6) >>`).
* Ao soltar a tecla, a mesa desacelera e para. Segurar a tecla para fazer o Jog não altera a distância digitada na tela.

### 5. Ajuste de Velocidade em Hz (`0`)
* Com a máquina parada, **mantenha a tecla `0` pressionada por mais de 2 segundos**.
* Enquanto estiver segurando a tecla `0`, a velocidade (`Vel :` no display) aumentará de `500 em 500 Hz` a cada meio segundo (de `500 Hz` até `6000 Hz`, retornando a `500 Hz` após o limite), sem adicionar zeros à distância configurada.

---

## 📚 Bibliotecas Necessárias
Certifique-se de instalar as seguintes bibliotecas na Arduino IDE:
1. `LiquidCrystal I2C` (por Frank de Brabander)
2. `FastAccelStepper` (por Jochen Krapf)
3. `Keypad` (por Mark Stanley, Alexander Brevig)
