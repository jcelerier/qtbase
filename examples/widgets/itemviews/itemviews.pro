TEMPLATE      = subdirs
SUBDIRS       = addressbook \
                basicsortfiltermodel \
                chart \
                coloreditorfactory \
                combowidgetmapper \
                customsortfiltermodel \
                dirview \
                editabletreemodel \
                fetchmore \
                frozencolumn \
                interview \
                pixelator \
                simpledommodel \
                simpletreemodel \
                simplewidgetmapper \
                spinboxdelegate \
                spreadsheet \
                stardelegate
!qtHaveModule(xml): SUBDIRS -= simpledommodel
