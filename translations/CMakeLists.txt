# cmake-format: off
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2020 by The qTox Project Contributors
# Copyright © 2024-2025 The TokTok team
# cmake-format: on

# Edit this list if translations are added
qt6_add_translation(
  translations_FILES
  ar.ts
  ber.ts
  be.ts
  bg.ts
  bn.ts
  cs.ts
  da.ts
  de.ts
  el.ts
  en.ts
  eo.ts
  es.ts
  et.ts
  fa.ts
  fi.ts
  fr.ts
  gl.ts
  he.ts
  hr.ts
  hu.ts
  is.ts
  it.ts
  ja.ts
  jbo.ts
  kn.ts
  ko.ts
  lt.ts
  lv.ts
  mk.ts
  nb_NO.ts
  nl.ts
  nl_BE.ts
  pl.ts
  pr.ts
  pt_BR.ts
  pt.ts
  ro.ts
  ru.ts
  si.ts
  sk.ts
  sl.ts
  sq.ts
  sr_Latn.ts
  sr.ts
  sv.ts
  sw.ts
  ta.ts
  tr.ts
  ug.ts
  uk.ts
  ur.ts
  vi.ts
  zh_CN.ts
  zh_TW.ts)

# Generate a resource file which contains all translation files
function(generate_translation_resource)
  file(
    WRITE "${CMAKE_CURRENT_BINARY_DIR}/translations.qrc.in"
    "<!DOCTYPE RCC>
    <RCC version=\"1.0\">
      <qresource prefix=\"/translations\">
    ")
  foreach(qm ${translations_FILES})
    get_filename_component(qm_name ${qm} NAME)
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/translations.qrc.in"
         "    <file alias=\"${qm_name}\">${qm}</file>\n")
  endforeach(qm)
  file(
    APPEND "${CMAKE_CURRENT_BINARY_DIR}/translations.qrc.in"
    "  </qresource>
    </RCC>
    ")
endfunction()

generate_translation_resource()
configure_file(${CMAKE_CURRENT_BINARY_DIR}/translations.qrc.in
               ./translations.qrc COPYONLY)

add_library(translations_library STATIC
            ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc)

# An explicit dependency is needed or AUTORCC will run before the translation
# files are created
set_target_properties(translations_library PROPERTIES AUTOGEN_TARGET_DEPENDS
                                                      "${translations_FILES}")
