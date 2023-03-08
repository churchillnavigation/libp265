set(HEADERS
    bitstream.h
    context.h
    libp265.h
    md5.h
    nal-parser.h
    nal.h
    pps.h
    refpic.h
    scan.h
    sei.h
    sps.h
    threads.h
    util.h
    vps.h
    vui.h
)

install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libp265)