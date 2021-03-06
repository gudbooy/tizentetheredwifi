# for i18n

SET(POFILES ar.po az.po bg.po ca.po cs.po da.po de.po el_GR.po en.po en_PH.po
	en_US.po es_ES.po es_US.po et.po eu.po fi.po fr.po fr_CA.po ga.po gl.po
	hi.po hr.po hu.po hy.po is.po it_IT.po ja_JP.po ka.po kk.po lt.po
	lv.po mk.po nb.po nl.po pl.po pt_BR.po pt_PT.po ro.po ru_RU.po sk.po
	sl.po sr.po sv.po tr_TR.po uk.po uz.po zh_CN.po zh_HK.po zh_TW.po ko_KR.po
   )

SET(MSGFMT "/usr/bin/msgfmt")

SET(WIFI_UG_PROJECT_NAME "ug-wifi-efl-UG")
SET(WIFI_QS_PROJECT_NAME "wifi-qs")

FOREACH(pofile ${POFILES})
	SET(pofile ${CMAKE_CURRENT_SOURCE_DIR}/${pofile})
	MESSAGE("PO: ${pofile}")
	GET_FILENAME_COMPONENT(absPofile ${pofile} ABSOLUTE)
	GET_FILENAME_COMPONENT(lang ${absPofile} NAME_WE)
	SET(moFile ${CMAKE_CURRENT_BINARY_DIR}/${lang}.mo)
	ADD_CUSTOM_COMMAND(
		OUTPUT ${moFile}
		COMMAND ${MSGFMT} -o ${moFile} ${absPofile}
		DEPENDS ${absPofile}
	)

	# install for UG
	INSTALL(FILES ${moFile}
		DESTINATION /usr/share/locale/${lang}/LC_MESSAGES RENAME ${WIFI_UG_PROJECT_NAME}.mo)

	# install for QS
	INSTALL(FILES ${moFile}
		DESTINATION /usr/share/locale/${lang}/LC_MESSAGES RENAME ${WIFI_QS_PROJECT_NAME}.mo)

	SET(moFiles ${moFiles} ${moFile})
ENDFOREACH(pofile)

MESSAGE(".mo files: ${moFiles}")
ADD_CUSTOM_TARGET(po ALL DEPENDS ${moFiles})
