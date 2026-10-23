import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

# 1. Configuração de caminhos e estilo
pasta_processados = os.path.join('..', 'processed')
pasta_imagens = 'img'
sns.set_theme(style="whitegrid")

if not os.path.exists(pasta_imagens):
    os.makedirs(pasta_imagens)

def carregar_dados(nome_arquivo):
    caminho = os.path.join(pasta_processados, nome_arquivo)
    if os.path.exists(caminho):
        df = pd.read_csv(caminho, sep=';')
        df['TIME'] = pd.to_datetime(df['TIME'])
        return df.sort_values('TIME')
    else:
        print(f"Aviso: Arquivo {nome_arquivo} não encontrado.")
        return None

def adicionar_estatisticas(ax, serie):
    """Adiciona linhas horizontais de Máx, Média e Mín no eixo informado."""
    vmin, vmed, vmax = serie.min(), serie.mean(), serie.max()
    
    ax.axhline(vmax, color='#C0392B', linestyle=':', alpha=0.7, label=f'Máx: {vmax:.1f}')
    ax.axhline(vmed, color='#7F8C8D', linestyle='--', alpha=0.7, label=f'Média: {vmed:.1f}')
    ax.axhline(vmin, color='#27AE60', linestyle=':', alpha=0.7, label=f'Mín: {vmin:.1f}')

# 2. Carregar os DataFrames
df_temp = carregar_dados('net_temp.csv')
df_hum  = carregar_dados('net_hum.csv')
df_pres = carregar_dados('net_press.csv')

# 3. Criar a figura - TAMANHO AJUSTADO (Largura, Altura)
# Reduzi de 14 para 8 a altura para ficar mais "compacto"
#fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(10, 8), sharex=True)
fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(20, 7), sharex=True)

# --- GRÁFICO 1: TEMPERATURA ---
if df_temp is not None:
    col = 'Temperatura Média'
    ax1.plot(df_temp['TIME'], df_temp[col], color='#E67E22', linewidth=1.5, label='Média')
    adicionar_estatisticas(ax1, df_temp[col])
    ax1.set_ylabel('Temp. (°C)')
    ax1.legend(loc='upper left', bbox_to_anchor=(1, 1), fontsize=8)

# --- GRÁFICO 2: UMIDADE ---
if df_hum is not None:
    col = 'Umidade Média'
    ax2.plot(df_hum['TIME'], df_hum[col], color='#2980B9', linewidth=1.5, label='Média')
    adicionar_estatisticas(ax2, df_hum[col])
    ax2.set_ylabel('Umid. (%)')
    ax2.legend(loc='upper left', bbox_to_anchor=(1, 1), fontsize=8)

# --- GRÁFICO 3: PRESSÃO ---
if df_pres is not None:
    col = 'Pressão Média'
    ax3.plot(df_pres['TIME'], df_pres[col], color='#C0392B', linewidth=1.5, label='Média')
    adicionar_estatisticas(ax3, df_pres[col])
    ax3.set_ylabel('Pres. (Pa)')
    ax3.set_xlabel('Tempo (Data/Hora)')
    ax3.legend(loc='upper left', bbox_to_anchor=(1, 1), fontsize=8)

# Ajustes de fontes e rotação para caber no espaço menor
plt.xticks(rotation=30, fontsize=9)

# Ajuste do layout - rect ajustado para as legendas menores
#plt.tight_layout(rect=[0, 0, 0.88, 1])
plt.tight_layout(rect=[0, 0, 0.94, 1])

# 4. Salvar e Exibir
caminho_saida = os.path.join(pasta_imagens, 'monitoramento_pth_compacto.png')
fig.savefig(caminho_saida, dpi=300, bbox_inches='tight')

print(f"Sucesso! Gráfico compacto salvo em: {caminho_saida}")
plt.show()