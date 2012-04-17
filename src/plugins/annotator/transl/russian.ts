<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="ru_RU">
<context>
    <name>FindAnnotationCollocationsDialog</name>
    <message>
        <location filename="../src/ui/FindAnnotationCollocationsDialog.ui" line="13"/>
        <source>dialog_tittle</source>
        <translation>Поиск сгруппировавнных аннотаций</translation>
    </message>
    <message>
        <location filename="../src/ui/FindAnnotationCollocationsDialog.ui" line="82"/>
        <source>region_size_label</source>
        <translation>Размер региона:</translation>
    </message>
    <message>
        <location filename="../src/ui/FindAnnotationCollocationsDialog.ui" line="124"/>
        <source>search_button</source>
        <translation>Искать</translation>
    </message>
    <message>
        <location filename="../src/ui/FindAnnotationCollocationsDialog.ui" line="148"/>
        <source>save_annotations_button</source>
        <translation>Сохранить как аннотации</translation>
    </message>
    <message>
        <location filename="../src/ui/FindAnnotationCollocationsDialog.ui" line="155"/>
        <source>clear_results_button</source>
        <translation>Сбросить результаты</translation>
    </message>
    <message>
        <location filename="../src/ui/FindAnnotationCollocationsDialog.ui" line="188"/>
        <source>cancel_button</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <location filename="../src/ui/FindAnnotationCollocationsDialog.ui" line="44"/>
        <source>1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/FindAnnotationCollocationsDialog.ui" line="49"/>
        <source>2</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/FindAnnotationCollocationsDialog.ui" line="168"/>
        <source>whole annotations</source>
        <translation>Аннотации не должны пересекать регион</translation>
    </message>
</context>
<context>
    <name>U2::AnnotatorPlugin</name>
    <message>
        <location filename="../src/AnnotatorPlugin.cpp" line="53"/>
        <source>dna_annotator_plugin</source>
        <translation>Поиск сгруппированных аннотаций</translation>
    </message>
    <message>
        <location filename="../src/AnnotatorPlugin.cpp" line="53"/>
        <source>dna_annotator_plugin_desc</source>
        <translation>Поиск сгруппированных аннотаций</translation>
    </message>
</context>
<context>
    <name>U2::AnnotatorViewContext</name>
    <message>
        <location filename="../src/AnnotatorPlugin.cpp" line="83"/>
        <source>Find annotated regions...</source>
        <translation>Найти сгруппированные аннотации...</translation>
    </message>
    <message>
        <location filename="../src/AnnotatorPlugin.cpp" line="101"/>
        <source>warning</source>
        <translation>Информация</translation>
    </message>
    <message>
        <location filename="../src/AnnotatorPlugin.cpp" line="101"/>
        <source>no_annotations_found</source>
        <translation>Ничего не нашли</translation>
    </message>
</context>
<context>
    <name>U2::CollocationSearchTask</name>
    <message>
        <location filename="../src/CollocationsDialogController.cpp" line="308"/>
        <location filename="../src/CollocationsDialogController.cpp" line="333"/>
        <source>collocation_search_task</source>
        <translation>Поиск сгруппированных аннотаций</translation>
    </message>
</context>
<context>
    <name>U2::CollocationsDialogController</name>
    <message>
        <location filename="../src/CollocationsDialogController.cpp" line="58"/>
        <source>click_to_add_new_annotation</source>
        <translation>&lt;&lt; Кликните &apos;+&apos; чтобы добавить аннотацию &gt;&gt;</translation>
    </message>
    <message>
        <location filename="../src/CollocationsDialogController.cpp" line="92"/>
        <source>stop</source>
        <translation>Остановить</translation>
    </message>
    <message>
        <location filename="../src/CollocationsDialogController.cpp" line="92"/>
        <source>cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <location filename="../src/CollocationsDialogController.cpp" line="98"/>
        <source>searching__found_%1_items_progress_%2</source>
        <translation>Сканируется последовательность, прогресс %2. Найдено %1 регионов.</translation>
    </message>
    <message>
        <location filename="../src/CollocationsDialogController.cpp" line="100"/>
        <source>found_%1_items</source>
        <translation>Найдено %1</translation>
    </message>
    <message>
        <location filename="../src/CollocationsDialogController.cpp" line="102"/>
        <source>ready</source>
        <translation>Готов к поиску</translation>
    </message>
    <message>
        <location filename="../src/CollocationsDialogController.cpp" line="102"/>
        <source>select_annotations</source>
        <translation>Выберите аннотации</translation>
    </message>
    <message>
        <location filename="../src/CollocationsDialogController.cpp" line="121"/>
        <source>no_more_annotations_left</source>
        <translation>Аннотаций больше нет.</translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::CollocationPrompter</name>
    <message>
        <location filename="../src/CollocationWorker.cpp" line="130"/>
        <source> sequence from &lt;u&gt;%1&lt;/u&gt;</source>
        <translation>последовательности из &lt;u&gt;%1&lt;/u&gt;</translation>
    </message>
    <message>
        <location filename="../src/CollocationWorker.cpp" line="133"/>
        <source> set of annotations from &lt;u&gt;%1&lt;/u&gt;</source>
        <translation>набора аннотаций из &lt;u&gt;%1&lt;/u&gt;</translation>
    </message>
    <message>
        <location filename="../src/CollocationWorker.cpp" line="140"/>
        <source>For each %1 and %2,</source>
        <translation>Для каждых %1 и %2,</translation>
    </message>
    <message>
        <location filename="../src/CollocationWorker.cpp" line="142"/>
        <source>For each %1%2,</source>
        <translation>Для каждого %1%2,</translation>
    </message>
    <message>
        <location filename="../src/CollocationWorker.cpp" line="158"/>
        <source> Annotations themselves may not span beyond the region.</source>
        <translation>целиком</translation>
    </message>
    <message>
        <location filename="../src/CollocationWorker.cpp" line="162"/>
        <source>%1 look if &lt;u&gt;%2&lt;/u&gt; annotations appear collocated within same region of length &lt;u&gt;%3&lt;/u&gt;.%4&lt;br&gt;Output the list of found regions annotated as &lt;u&gt;%5&lt;/u&gt;.</source>
        <translation>%1 искать группы аннотаций &lt;u&gt;%2&lt;/u&gt; расположенные &lt;u&gt;%4&lt;/u&gt; внутри регионов не длиннее &lt;u&gt;%3&lt;/u&gt;.&lt;br&gt;Выдать список найденных групп в виде аннотаций &quot;&lt;u&gt;%5&lt;/u&gt;&quot;.</translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::CollocationWorker</name>
    <message>
        <location filename="../src/CollocationWorker.cpp" line="234"/>
        <source>Found %1 collocations</source>
        <translation>Найдено %1 групп</translation>
    </message>
    <message>
        <location filename="../src/CollocationWorker.cpp" line="94"/>
        <source>Group annotations</source>
        <translation>Найденные регионы</translation>
    </message>
    <message>
        <location filename="../src/CollocationWorker.cpp" line="99"/>
        <source>Result annotation</source>
        <translation>Имя аннотации</translation>
    </message>
    <message>
        <location filename="../src/CollocationWorker.cpp" line="100"/>
        <source>Group of annotations</source>
        <translation>Аннотации в группе</translation>
    </message>
    <message>
        <location filename="../src/CollocationWorker.cpp" line="101"/>
        <source>Region size</source>
        <translation>Размер региона</translation>
    </message>
    <message>
        <location filename="../src/CollocationWorker.cpp" line="102"/>
        <source>Must fit into region</source>
        <translation>Компактные группы</translation>
    </message>
    <message>
        <location filename="../src/CollocationWorker.cpp" line="109"/>
        <source>Collocation Search</source>
        <translation>Поиск групп аннотаций</translation>
    </message>
    <message>
        <location filename="../src/CollocationWorker.cpp" line="67"/>
        <source>At least 2 annotations are required for collocation search.</source>
        <translation>Необходимо указать не менее 2-х различных имён аннотаций для поиска.</translation>
    </message>
    <message>
        <location filename="../src/CollocationWorker.cpp" line="94"/>
        <source>Annotated regions containing found collocations.</source>
        <translation>Список аннотаций отмечающих найденные группы аннотаций.</translation>
    </message>
    <message>
        <location filename="../src/CollocationWorker.cpp" line="99"/>
        <source>Name of the result annotations to mark found collocations</source>
        <translation>Имя результирующих аннотаций отмечающих найденные группы</translation>
    </message>
    <message>
        <location filename="../src/CollocationWorker.cpp" line="100"/>
        <source>A list of annotation names to search. Found regions will contain all the named annotations.</source>
        <translation>Список имён интересующих аннотаций в группе. Ищутся регионы содержащие все указанные аннотации.</translation>
    </message>
    <message>
        <location filename="../src/CollocationWorker.cpp" line="101"/>
        <source>Effectively this is the maximum allowed distance between the interesting annotations in a group</source>
        <translation>Фактически, это максимальная допустимая дистанция между аннотациями в одной группе.</translation>
    </message>
    <message>
        <location filename="../src/CollocationWorker.cpp" line="102"/>
        <source>Whether the interesting annotations should entirely fit into the specified region to form a group</source>
        <translation>Аннотации в группе должны целиком входить в указанный регион - т.е. дистанция в группе считается по внешним границам аннотаций.</translation>
    </message>
    <message>
        <location filename="../src/CollocationWorker.cpp" line="110"/>
        <source>Finds groups of specified annotations in each supplied set of annotations, stores found regions as annotations.</source>
        <translation>Поиск сгруппированных аннотаций. Ищутся регионы последовательности, содержащие все аннотации из заданного списка, расположенные друг от друга на расстоянии не более заданного.</translation>
    </message>
    <message>
        <location filename="../src/CollocationWorker.cpp" line="89"/>
        <source>Input data</source>
        <translation>Входные данные</translation>
    </message>
    <message>
        <location filename="../src/CollocationWorker.cpp" line="90"/>
        <source>An input sequence and a set of annotations to search in.</source>
        <translation>Входные последовательность и набор аннотированных регионов для поиска.</translation>
    </message>
</context>
</TS>
