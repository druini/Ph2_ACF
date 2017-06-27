#!/bin/bash
export SESSION_NAME=irradtest
#direcories
export TMUX_BASE_DIR=$(pwd)
export TMUX_USB_DIR=$TMUX_BASE_DIR/../Ph2_USBInstDriver
#tmux panes
export HMP_PANE=1
export KE_PANE=2
export MAIN_PANE=0
#HMP ports
export HMP_HTTP_PORT=8080
export HMP_ZMQ_PORT=8081
#KE ports
export KE_HTTP_PORT=8082
export KE_ZMQ_PORT=8083
#monitoring interval
export INTERVAL=2
#interval at which to take bias sweeps
export SWEEPINTERVAL=2
#interval at which to re-start the monitoring
export RESTARTINTERVAL=3
export TIMEOUT=1200

#params: 1 pane, 2 working dir
function setup_env {
    tmux select-window -t $SESSION_NAME
    tmux select-pane -t $1
    tmux send-keys -t $SESSION_NAME "cd $2" C-m
    tmux send-keys -t $SESSION_NAME ". $2/setup.sh" C-m
}

function start_HMP {
        tmux select-window -t $SESSION_NAME
        tmux select-pane -t $HMP_PANE
        tmux send-keys -t $SESSION_NAME "cd $TMUX_USB_DIR" C-m
        tmux send-keys -t $SESSION_NAME ". $TMUX_USB_DIR/setup.sh" C-m
        tmux send-keys -t $SESSION_NAME "lvSupervisor -f $TMUX_USB_DIR/settings/HMP4040.xml -r $HMP_ZMQ_PORT -p $HMP_HTTP_PORT -i $INTERVAL -S" C-m
}

function start_KE {
        tmux select-window -t $SESSION_NAME
        tmux select-pane -t $KE_PANE
        tmux send-keys -t $SESSION_NAME "cd $TMUX_USB_DIR" C-m
        tmux send-keys -t $SESSION_NAME ". $TMUX_USB_DIR/setup.sh" C-m
        tmux send-keys -t $SESSION_NAME "dmmSupervisor -r $KE_ZMQ_PORT -p $KE_HTTP_PORT -i $INTERVAL" C-m
}

function restart_HMP {
        tmux select-window -t $SESSION_NAME
        tmux select-pane -t $HMP_PANE
        tmux send-keys -t $SESSION_NAME "e" C-m
        sleep 2
        tmux send-keys -t $SESSION_NAME "q" C-m
        sleep 2
        tmux send-keys -t $SESSION_NAME C-c
        sleep 2
        tmux send-keys -t $SESSION_NAME "lvSupervisor -f $TMUX_USB_DIR/settings/HMP4040.xml -r $HMP_ZMQ_PORT -p $HMP_HTTP_PORT -i $INTERVAL -S" C-m
}

function restart_KE {
        tmux select-window -t $SESSION_NAME
        tmux select-pane -t $KE_PANE
        tmux send-keys -t $SESSION_NAME "e" C-m
        sleep 2
        tmux send-keys -t $SESSION_NAME "q" C-m
        sleep 2
        tmux send-keys -t $SESSION_NAME C-c
        sleep 2
        tmux send-keys -t $SESSION_NAME "dmmSupervisor -r $KE_ZMQ_PORT -p $KE_HTTP_PORT -i $INTERVAL" C-m
}

function run_test {
    while true; do
        echo "Running iteration $CYCLECOUNT ..."

        if (($CYCLECOUNT % $SWEEPINTERVAL == 0)); then
            echo 'running full cycle with bias sweeps and Stub Scan ...'
            # -v 0 is just a dummy as the actual threshold values are hardcoded
            # this just runs calibration, noise measurement and takes data
            $TMUX_BASE_DIR/bin/cbc3irrad -s -v 0 --timout $TIMEOUT -b | tee $TMUX_BASE_DIR/consoledump.log
        else
            echo 'running cycle without bias sweep...'
            # -v 0 is just a dummy as the actual threshold values are hardcoded
            # --full runs the bias sweeps, calibration, noise measurement and stub sweep
            $TMUX_BASE_DIR/bin/cbc3irrad -s -v 0 -b --full | tee $TMUX_BASE_DIR/consoledump.log
        fi

        echo "$CYCLECOUNT iteration of bin/cbc3irrad finished with exit code $?. Respawning..." | tee $TMUX_BASE_DIR/consoledump.log

        if (($CYCLECOUNT % $RESTARTINTERVAL == 0)); then 
            echo "Re-starting monitoring servers"
            restart_HMP
            restart_KE
        fi

        CYCLECOUNT=$((CYCLECOUNT+1))
        sleep 30
    done
}

export -f start_HMP
export -f start_KE
export -f restart_HMP
export -f restart_KE
export -f run_test


function run_irradtest {
    tmux select-window -t $SESSION_NAME
    tmux select-pane -t $MAIN_PANE
    tmux send-keys -t $SESSION_NAME "export CYCLECOUNT=0" C-m
    tmux send-keys -t $SESSION_NAME "run_test" C-m
}

#default pane (left) is 0
#upper right is pane 1
#lower right is pane 2
tmux list-session 2>&1 | grep -q "^$SESSION_NAME" || tmux new-session -s $SESSION_NAME -d
tmux rename-window 'overview'
#check if 3 panes already exist
npanes=$(tmux list-panes | wc -l)
if (($npanes == 1)); then
    tmux split-window -h 
    tmux split-window -v
elif(($npanes == 2)); then
    tmux select-pane -t 1
    tmux split-window -v
elif (($npanes == 3)); then
    echo "enough panes available!"
fi

# cd to Ph2_ACF directory in left pane and start Monitoring servers in the right panes 1/2
setup_env 0 $TMUX_BASE_DIR
start_HMP 
start_KE
run_irradtest
