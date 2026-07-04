
/*
Nota: Se você usa o Arduino IDE padrão, a função vTaskList (para memória) funcionará
nativamente, mas a vTaskGetRunTimeStats (para tempo de CPU) pode retornar vazia ou falhar
dependendo da versão do Core do ESP32 instalada, pois exige a recompilação do FreeRTOS
com suporte a timers de alta resolução.
*/

// =====================================================
// Imprimir Diagnostico do Sistema
// =====================================================
#include <Arduino.h>

// Variáveis para guardar o "endereço" de cada tarefa
TaskHandle_t handleTarefa1 = NULL;
TaskHandle_t handleTarefa2 = NULL;


// Protótipos das funções
void taskExemploRAM(void *pvParameters);
void taskExemploCPU(void *pvParameters);
void imprimirDiagnosticoSistema();

void setup() {
  Serial.begin(115200);
  delay(1000); // Aguarda a estabilização do monitor serial
  Serial.println("\n[SISTEMA] Inicializando Monitor de Diagnóstico...");

  // Correção: Removido o '1' extra que estava duplicando o Core ID
  xTaskCreatePinnedToCore(taskExemploCPU, "Task_CPU", 3072, NULL, 1, &handleTarefa1, 1);
  xTaskCreatePinnedToCore(taskExemploCPU, "Task_CPU", 3072, NULL, 1, &handleTarefa2, 1);

}

void loop() {
  // Imprime o relatório completo de diagnóstico
  imprimirDiagnosticoSistema();
  
  // Aguarda 5 segundos de forma não-bloqueante para a loop task
  vTaskDelay(pdMS_TO_TICKS(5000));
}

// Task de Teste 1: Consome pouca CPU, serve para monitorar o uso base de RAM
void taskExemploRAM(void *pvParameters) {
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// Task de Teste 2: Executa cálculos matemáticos para gerar carga de CPU
void taskExemploCPU(void *pvParameters) {
  volatile double calculo = 0;
  for (;;) {
    for (int i = 0; i < 100000; i++) {
      calculo = sin(i) * cos(i);
    }
    // Pausa curta para permitir que o IDLE do FreeRTOS respire
    vTaskDelay(pdMS_TO_TICKS(100)); 
  }
}

// Função responsável por extrair e exibir os dados do FreeRTOS
void imprimirDiagnosticoSistema() {
    Serial.println("\n==================================================");
    Serial.println("       MONITORAMENTO DE TAREFAS (FREE RAM)   ");
    Serial.println("==================================================");
    
    Serial.printf("Memória RAM Livre Atual (Heap):       %u bytes\n", ESP.getFreeHeap());
    Serial.printf("Menor nível de RAM já atingido:       %u bytes\n", ESP.getMinFreeHeap());
    Serial.printf("Maior bloco contínuo de RAM:          %u bytes\n", ESP.getMaxAllocHeap());
    Serial.println("--------------------------------------------------");

    // 2. Diagnóstico da Tarefa 1
    if (handleTarefa1 != NULL) {
        unsigned int stackTarefa1 = uxTaskGetStackHighWaterMark(handleTarefa1);
        Serial.printf("Folga na Stack da Tarefa 1 (Sensores):  %u bytes\n", stackTarefa1);
        if (stackTarefa1 < 200) Serial.println("⚠️ ALERTA: Tarefa 1 está quase sem memória!");
    } else {
        Serial.println("Tarefa 1 não foi inicializada.");
    }

    // 3. Diagnóstico da Tarefa 2
    if (handleTarefa2 != NULL) {
        unsigned int stackTarefa2 = uxTaskGetStackHighWaterMark(handleTarefa2);
        Serial.printf("Folga na Stack da Tarefa 2 (WiFi)     : %u bytes\n", stackTarefa2);
        if (stackTarefa2 < 200) Serial.println("⚠️ ALERTA: Tarefa 2 está quase sem memória!");
    } else {
        Serial.println("Tarefa 2 não foi inicializada.");
    }

        // 3. Informações de Hardware do Chip
    Serial.printf("\nModelo do Chip:                      ESP32 (Rev %d)\n", ESP.getChipRevision());
    Serial.printf("Quantidade de Cores:                 %d\n", ESP.getChipCores());
    Serial.printf("Frequência do Processador:           %u MHz\n", ESP.getCpuFreqMHz());

    Serial.println("==================================================\n");
}