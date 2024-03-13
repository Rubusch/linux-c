## setup my home labgrid environment
##
## cd into the labgrid folder, then source this file...
## labgrid-coordinator:       10.1.10.1
## labgrid-exporter (place):  rpi
alias lc='labgrid-client'

export LG_ENV=$(pwd)/rpi5__shell__env.yaml
export LG_PLACE=rpi
