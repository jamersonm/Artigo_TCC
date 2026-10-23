import pandas as pd
import os
from datetime import datetime

# 1. Configuração de Caminhos
pasta_entrada = os.path.join('..', 'raw')
pasta_saida = os.path.join('..', 'processed')

# 2. Definição das datas de corte (Início e Fim)
data_inicio = datetime(2026, 3, 31, 11, 30, 0)
data_fim    = datetime(2026, 4, 7, 12, 00, 0)

if not os.path.exists(pasta_saida):
    os.makedirs(pasta_saida)

# 3. Listar arquivos net_
arquivos_net = [f for f in os.listdir(pasta_entrada) if f.startswith('net_') and f.endswith('.csv')]

print(f"Iniciando processamento de {len(arquivos_net)} arquivos...")

for arquivo in arquivos_net:
    caminho_completo = os.path.join(pasta_entrada, arquivo)
    
    # 4. Carregar o CSV
    df = pd.read_csv(caminho_completo, sep=',')
    
    # Limpeza de Duplicatas
    df = df.drop_duplicates().copy()
    
    # Limpar nomes das colunas
    df.columns = [c.strip() for c in df.columns]
    
    # 5. Localizar e padronizar a coluna de Tempo
    col_tempo_original = next((c for c in df.columns if c.lower() == 'time'), None)
    
    if col_tempo_original:
        df = df.rename(columns={col_tempo_original: 'TIME'})
    else:
        print(f"Aviso: Coluna 'time' não encontrada em {arquivo}. Pulando...")
        continue

    # 6. Converter e Ordenar o Tempo
    df['TIME'] = pd.to_datetime(df['TIME'], errors='coerce')
    df = df.sort_values('TIME').dropna(subset=['TIME'])
    
    # 7. Limpar as colunas de DADOS
    cols_dados = [c for c in df.columns if c != 'TIME']
    
    for col in cols_dados:
        df[col] = df[col].astype(str).str.replace(r'[^0-9.,-]', '', regex=True)
        df[col] = df[col].str.replace(',', '.')
        df[col] = pd.to_numeric(df[col], errors='coerce')

    # 8. Filtrar o INTERVALO (Início e Fim)
    # Seleciona apenas as linhas que estão entre as duas datas
    df_filtrado = df[(df['TIME'] >= data_inicio) & (df['TIME'] <= data_fim)].copy()
    
    # 9. Renomear o arquivo e salvar
    novo_nome = arquivo.replace('_raw', '')
    caminho_saida_final = os.path.join(pasta_saida, novo_nome)
    
    # Salvando com ';' para padronizar
    df_filtrado.to_csv(caminho_saida_final, index=False, sep=';')
    
    print(f"Sucesso: {novo_nome} | Linhas no intervalo: {len(df_filtrado)}")

print("\nProcessamento finalizado. Dados filtrados pelo intervalo definido!")