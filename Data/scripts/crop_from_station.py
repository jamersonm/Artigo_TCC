import pandas as pd
import os

# 1. Configuração de Caminhos
caminho_entrada = os.path.join('..', 'processed', 'station_time_fixed_and_filtered.csv')
pasta_saida = os.path.join('..', 'processed')

# Garante que a pasta de saída existe
if not os.path.exists(pasta_saida):
    os.makedirs(pasta_saida)
    print(f"Pasta de saída verificada/criada em: {pasta_saida}")

# 2. Carregar os dados (Apenas uma vez)
if not os.path.exists(caminho_entrada):
    print(f"Erro crítico: O arquivo {caminho_entrada} não foi encontrado.")
else:
    df = pd.read_csv(caminho_entrada, sep=';')

    # 3. Função auxiliar para salvar com o prefixo "station_"
    def salvar_crop(df_original, colunas, nome_base):
        df_cortado = df_original[colunas]
        nome_arquivo = f"station_{nome_base}"
        caminho_final = os.path.join(pasta_saida, nome_arquivo)
        df_cortado.to_csv(caminho_final, index=False, sep=';')
        print(f"Sucesso: {nome_arquivo} gerado.")

    print("--- Iniciando Extração de Dados Ambientais ---")
    
    # 4. Extração de Grupos Ambientais
    grupos_ambientais = {
        'temp_raw.csv': ['TIME', 'VOC_TEMP', 'H2S_TEMP', 'SO2_TEMP', 'NH3_TEMP', 'CH3SH_TEMP', 'BME_TEMP'],
        'hum_raw.csv': ['TIME', 'VOC_HUM', 'H2S_HUM', 'SO2_HUM', 'NH3_HUM', 'CH3SH_HUM', 'BME_HUM'],
        'press_raw.csv': ['TIME', 'BME_PRESS'],
        'anemo_raw.csv': ['TIME', 'ANEMO_DIR', 'ANEMO_VEL']
    }

    for nome_arq, colunas in grupos_ambientais.items():
        salvar_crop(df, colunas, nome_arq)

    print("\n--- Iniciando Extração de Gases (PPB/UG) ---")

    # 5. Extração de Gases Individuais
    gases = ['VOC', 'H2S', 'SO2', 'NH3', 'CH3SH']

    for gas in gases:
        colunas_gas = ['TIME', f'{gas}_PPB', f'{gas}_UG']
        # Nome base para a função adicionar o prefixo depois
        nome_base_gas = f'{gas.lower()}_raw.csv'
        salvar_crop(df, colunas_gas, nome_base_gas)

    print("\n" + "="*40)
    print("PROCESSO CONCLUÍDO COM SUCESSO")
    print("="*40)