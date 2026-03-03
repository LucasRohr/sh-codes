#!/usr/bin/env bash

# ============================================================
# dir_monitor.sh
# Monitora um diretório definido pela variável MON_DIR
# ============================================================

# Intervalo de polling em segundos
readonly POLLING_INTERVAL=2

# Controle para usar Parte 2 ou Parte 3 via flag
readonly USE_SILENT_MODE=true

# Função para logar erros no stderr
log_error() {
    echo "[ERRO] $*" >&2
}

# Função para validar diretório de monitoramento
# Retorna 0 se o diretório for válido, 1 caso contrário
validate_mon_dir() {
    # Verificar se a variável de ambiente MON_DIR está definida e não vazia
    # -z verifica se é vazia além de se existe
    if [ -z "$MON_DIR" ]; then
        log_error "A variável de ambiente MON_DIR não está definida ou está vazia."
        log_error "Uso: MON_DIR=/caminho/do/diretorio ./dir_monitor.sh"
        exit 1
    fi

    # -d verifica se é um diretório existente e acessível, necessário para monitorar
    if [ ! -d "$MON_DIR" ]; then
        log_error "O caminho '$MON_DIR' não existe ou não é um diretório."
        exit 1
    fi
}


# Função para listar as entradas no diretório
# Omite arquivos de backup e ordena resultados
list_entries() {
    # -r read (leitura)
    # -d '' delimita por ('\0')
    while IFS= read -r -d '' filepath; do
        local name

        # (basename -- "$filepath") retorna o nome do arquivo ou diretório sem o caminho
        name=$(basename -- "$filepath")

        # Omite arquivos de backup
        # nomes terminados em ~, .bak ou .old
        case "$name" in
            *~|*.bak|*.old)
                continue
                ;;
        esac

        # printf é mais seguro que echo para nomes com caracteres especiais
        # usado aqui para evitar problemas com esses caracteres
        printf '%s\n' "$name"

    # -maxdepth 1 limita a busca em um nível de profundidade
    # -mindepth 1 inicia a busca a partir do primeiro nível
    # -print0 imprime o resultado com null como delimitador
    # | sort ordena os resultados alfabeticamente
    done < <(find "$MON_DIR" -maxdepth 1 -mindepth 1 -print0) | sort
}

# Parte 2: Função para monitorar o diretório
# Verifica o diretório a cada intervalo de polling e imprime as entradas
monitor_dir_list() {
    while true; do
        echo "--- $(date '+%Y-%m-%d %H:%M:%S') | $MON_DIR ---" # Imprime data e hora e diretório monitorado
        list_entries # Lista as entradas no diretório (função auxiliar)
        echo ""

        sleep "$POLLING_INTERVAL" # Aguarda o intervalo de polling
    done
}

# Parte 3: Função para monitorar o diretório de forma silenciosa e com disparo de comando fornecido
# Verifica o diretório a cada intervalo de polling e executa o comando passado ao verificar atualização
monitor_dir_silent_updates() {
    # Salva snapshot anterior para comparação
    local old_snapshot
    old_snapshot=$(list_entries)

    while true; do
        # Aguarda o intervalo de polling
        sleep "$POLLING_INTERVAL"

        # Salva snapshot atual para comparação
        new_snapshot=$(list_entries)

        # Compara snapshots
        if [ "$old_snapshot" != "$new_snapshot" ]; then
            echo "[$(date '+%Y-%m-%d %H:%M:%S')] Alteração detectada em: $MON_DIR" >&2

            # Dispara o comando fornecido em background.
            # 'disown' remove o job da tabela do bash
            "$@" &
            disown

            # Atualiza o snapshot anterior para a próxima comparação
            old_snapshot="$new_snapshot"
        fi
    done
}


main() {
    # Validar diretório de monitoramento
    validate_mon_dir

    echo "Monitoramento iniciado em: $MON_DIR"
    echo "Intervalo de polling: ${POLLING_INTERVAL}s"

    if [ "$USE_SILENT_MODE" = true ]; then
        # Parte 3: Iniciar monitoramento silencioso das atualizações e disparo de comando fornecido

        # Se nenhum argumento de comando for fornecido, encerra com erro, pois é necessário para disparo de comando
        if [ $# -eq 0 ]; then
            log_error "Nenhum comando fornecido."
            log_error "Uso: MON_DIR=/caminho ./dir_monitor.sh <comando> [args...]"
            log_error "Exemplo: MON_DIR=/tmp/pasta ./dir_monitor.sh echo 'Mudança detectada'"
            exit 1
        fi

        # Caso contrário, passa o comando fornecido como argumento para a função
        monitor_dir_silent_updates "$@"
    else
        # Parte 2: Iniciar monitoramento com print das entradas no diretório
        monitor_dir_list
    fi
}

main "$@" # Passa todos os argumentos para a função main e executa o script