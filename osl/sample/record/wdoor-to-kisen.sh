#!/bin/sh

#
# その棋譜リストをもとに、.kifを作成
#
bjam release smp=off csa-to-kisen && \
cat $filename.files | ../../build/sample/record/gcc-4.3.3/release/address-model-64/architecture-x86/instruction-set-athlon64/qt3support-on/smp-off/csa-to-kisen -o $filename.kif 2>&1 | tee wdoor-to-kisen.log

exit 0
