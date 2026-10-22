# 假设安装在 /opt/arm/armpl_25.07_gcc-14.2 （你可用 ls /opt/arm 确认）
export ARMPL=/opt/arm/armpl_25.07_gcc/

export LD_LIBRARY_PATH=${ARMPL}/lib:${LD_LIBRARY_PATH}
export CPATH=${ARMPL}/include:${CPATH}
export LIBRARY_PATH=${ARMPL}/lib:${LIBRARY_PATH}

