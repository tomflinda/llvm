ARG base_tag=alldeps
ARG base_image=ghcr.io/intel/llvm/ubuntu2404_intel_drivers

FROM $base_image:$base_tag

USER root

COPY scripts/drivers_entrypoint.sh /drivers_entrypoint.sh
RUN mkdir -p /opt/sycl
ADD sycl_linux.tar.gz /opt/sycl/

ENV PATH /opt/sycl/bin:$PATH
ENV LD_LIBRARY_PATH /opt/sycl/lib:$LD_LIBRARY_PATH

USER sycl

ENTRYPOINT ["/bin/bash", "/drivers_entrypoint.sh"]

