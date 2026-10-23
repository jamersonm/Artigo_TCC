import pandas as pd
import os

# 1. Configuração do Caminho
caminho_arquivo = os.path.join('..', 'processed', 'net_press.csv')

if os.path.exists(caminho_arquivo):
    # 2. Carregar o CSV
    # Usamos sep=';' para bater com o seu formato
    df = pd.read_csv(caminho_arquivo, sep=';')

    # 3. Multiplicar as colunas por 10
    # Selecionamos as colunas que não são 'TIME'
    colunas_numericas = ['Pressão Média', 'Pressão Máxima', 'Pressão Mínima']
    
    # Realiza a multiplicação
    df[colunas_numericas] = df[colunas_numericas] * 10

    # 4. Sobreescrever o arquivo original
    # index=False evita que o pandas crie uma coluna extra de números
    df.to_csv(caminho_arquivo, index=False, sep=';')

    print(f"✅ Arquivo {caminho_arquivo} atualizado com sucesso!")
    print(df.head(2)) # Exibe as primeiras linhas para conferência
else:
    print(f"❌ Erro: Arquivo {caminho_arquivo} não encontrado.")