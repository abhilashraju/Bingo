FROM rabhil/abhi_ubuntu:latest


RUN set -ex \
    && mkdir work \
    && cd work \
    && git clone https://github.com/abhilashraju/Bongo.git \
    && cd Bongo\
    && conan profile detect\
    && conan create .

RUN set -ex \
    && cd work \
    && git clone https://github.com/abhilashraju/Bingo.git  

RUN set -ex\
    && echo ./work/Bingo/examples/build/aws_service/aws_sns_publisher /work/Bingo/examples/aws_service/resources 9003 > run.sh\
    && chmod 777 run.sh

RUN set -ex \
    && cd work/Bingo \
    && git pull 
    # && ./configure

CMD ["bash","-C"]




###############################################################################
# Cleanup
###############################################################################
# RUN set -ex \
#     && apt-get update -qq \
#     && apt-get clean \
#     && rm -rf /tmp/*
