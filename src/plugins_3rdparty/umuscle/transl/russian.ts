<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="ru_RU">
<context>
    <name>MuscleAlignmentDialog</name>
    <message>
        <location filename="../src/ui/MuscleAlignDialog.ui" line="14"/>
        <source>Align with MUSCLE</source>
        <translation>Выравнивание MUSCLE</translation>
    </message>
    <message>
        <location filename="../src/ui/MuscleAlignDialog.ui" line="28"/>
        <source>Mode</source>
        <translation>Конфигурация</translation>
    </message>
    <message>
        <location filename="../src/ui/MuscleAlignDialog.ui" line="40"/>
        <source>Mode details:</source>
        <translation>Описание:</translation>
    </message>
    <message>
        <location filename="../src/ui/MuscleAlignDialog.ui" line="54"/>
        <source>Advanced options</source>
        <translation>Дополнительные настройки</translation>
    </message>
    <message>
        <location filename="../src/ui/MuscleAlignDialog.ui" line="73"/>
        <source>Max iterations</source>
        <translation type="unfinished">Максимальное число итераций</translation>
    </message>
    <message>
        <location filename="../src/ui/MuscleAlignDialog.ui" line="112"/>
        <source>Max time (minutes)</source>
        <translation type="unfinished">Максимальное время работы (мин.)</translation>
    </message>
    <message>
        <location filename="../src/ui/MuscleAlignDialog.ui" line="157"/>
        <source>Region to align</source>
        <translation>Выравнивать:</translation>
    </message>
    <message>
        <source>Custom range</source>
        <translation type="obsolete">Указанный регион</translation>
    </message>
    <message>
        <location filename="../src/ui/MuscleAlignDialog.ui" line="243"/>
        <source>-</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/MuscleAlignDialog.ui" line="176"/>
        <source>Whole alignment</source>
        <translation>Целые последовательности</translation>
    </message>
    <message>
        <location filename="../src/ui/MuscleAlignDialog.ui" line="189"/>
        <source>Column range</source>
        <translation type="unfinished">Указанный регион столбцов</translation>
    </message>
    <message>
        <source>Remote run...</source>
        <translation type="obsolete">Удаленный запуск...</translation>
    </message>
    <message>
        <location filename="../src/ui/MuscleAlignDialog.ui" line="284"/>
        <source>Align</source>
        <translation>Выровнять</translation>
    </message>
    <message>
        <location filename="../src/ui/MuscleAlignDialog.ui" line="294"/>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <location filename="../src/ui/MuscleAlignDialog.ui" line="60"/>
        <source>Do not re-arrange sequences (-stable)</source>
        <translation>Сохранять порядок последовательностей (ключ -stable)</translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::MusclePrompter</name>
    <message>
        <location filename="../src/MuscleWorker.cpp" line="81"/>
        <source> from %1</source>
        <translation> из %1</translation>
    </message>
    <message>
        <location filename="../src/MuscleWorker.cpp" line="89"/>
        <source>Aligns each MSA supplied &lt;u&gt;%1&lt;/u&gt; with MUSCLE using &quot;&lt;u&gt;%2&lt;/u&gt;&quot; mode.</source>
        <translation>Для каждого выравнивания&lt;u&gt;%1&lt;/u&gt;, запустить MUSCLE в  &lt;u&gt;&quot;%2&quot;&lt;/u&gt; и выдать результат на  выход.</translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::MuscleWorker</name>
    <message>
        <location filename="../src/MuscleWorker.cpp" line="31"/>
        <source>Input MSA</source>
        <translation>Входное выравнивание</translation>
    </message>
    <message>
        <location filename="../src/MuscleWorker.cpp" line="33"/>
        <source>Multiple sequence alignment</source>
        <translation>Множественное выравнивание</translation>
    </message>
    <message>
        <source>MUSCLE alignment</source>
        <translation type="obsolete">Выравнивание MUSCLE</translation>
    </message>
    <message>
        <location filename="../src/MuscleWorker.cpp" line="42"/>
        <source>Mode</source>
        <translation>Конфигурация</translation>
    </message>
    <message>
        <location filename="../src/MuscleWorker.cpp" line="45"/>
        <source>Stable order</source>
        <translation>Сохранять порядок</translation>
    </message>
    <message>
        <location filename="../src/MuscleWorker.cpp" line="32"/>
        <source>Multiple sequence alignment to be processed.</source>
        <translation>Входные данные для выравнивания.</translation>
    </message>
    <message>
        <location filename="../src/MuscleWorker.cpp" line="33"/>
        <source>Result of alignment.</source>
        <translation>Результат выравнивания.</translation>
    </message>
    <message>
        <location filename="../src/MuscleWorker.cpp" line="52"/>
        <source>Align with MUSCLE</source>
        <translation type="unfinished">Выравнять с помощью MUSCLE</translation>
    </message>
    <message>
        <location filename="../src/MuscleWorker.cpp" line="53"/>
        <source>MUSCLE is public domain multiple alignment software for protein and nucleotide sequences.&lt;p&gt;&lt;dfn&gt;MUSCLE stands for MUltiple Sequence Comparison by Log-Expectation.&lt;/dfn&gt;&lt;/p&gt;</source>
        <translation>Пакет MUSCLE предназначен для выравнивания множественных протеиновых и нуклеотидных последовательностей.</translation>
    </message>
    <message>
        <location filename="../src/MuscleWorker.cpp" line="121"/>
        <source>An empty MSA has been supplied to MUSCLE.</source>
        <translation type="unfinished">MUSCLE получил пустое МВ.</translation>
    </message>
    <message>
        <location filename="../src/MuscleWorker.cpp" line="137"/>
        <source>Aligned %1 with MUSCLE</source>
        <translation>Выровнял %1 с MUSCLE</translation>
    </message>
    <message>
        <location filename="../src/MuscleWorker.cpp" line="43"/>
        <source>Selector of preset configurations, that give you the choice of optimizing accuracy, speed, or some compromise between the two. The default favors accuracy.</source>
        <translation>Выбор между режимами максимальной точности, скорости либо компромиса между ними. По умолчанию оптимизируется точность выравнивания.</translation>
    </message>
    <message>
        <location filename="../src/MuscleWorker.cpp" line="46"/>
        <source>Do not rearrange aligned sequences (-stable switch of MUSCLE). &lt;p&gt;Otherwise, MUSCLE re-arranges sequences so that similar sequences are adjacent in the output file. This makes the alignment easier to evaluate by eye. </source>
        <translation>Сохранять начальный порядок последовательностей в выравнивании. &lt;p&gt;Иначе, MUSCLE группирует похожие последовательности в выходном выравнивании, для удобства визуального сравнения.</translation>
    </message>
</context>
<context>
    <name>U2::MuscleAdapter</name>
    <message>
        <location filename="../src/MuscleAdapter.cpp" line="49"/>
        <source>No sequences in input file</source>
        <translation>Выравнивание не содержит данных</translation>
    </message>
    <message>
        <location filename="../src/MuscleAdapter.cpp" line="76"/>
        <source>alignment_is_empty</source>
        <translation>Выравнивание не содержит данных</translation>
    </message>
    <message>
        <location filename="../src/MuscleAdapter.cpp" line="27"/>
        <location filename="../src/MuscleAdapter.cpp" line="175"/>
        <location filename="../src/MuscleAdapter.cpp" line="260"/>
        <location filename="../src/MuscleAdapter.cpp" line="442"/>
        <source>Internal MUSCLE error: %1</source>
        <translation>Внутренняя ошибка MUSCLE: %1</translation>
    </message>
    <message>
        <location filename="../src/MuscleAdapter.cpp" line="253"/>
        <source>Invalid input alignment</source>
        <translation>Некорректное входное выравнивание</translation>
    </message>
    <message>
        <location filename="../src/MuscleAdapter.cpp" line="269"/>
        <location filename="../src/MuscleAdapter.cpp" line="451"/>
        <source>Incompatible alphabets</source>
        <translation>Несовместимые алфавиты</translation>
    </message>
    <message>
        <location filename="../src/MuscleAdapter.cpp" line="298"/>
        <source>Aligning profiles</source>
        <translation>Выравнивание</translation>
    </message>
    <message>
        <location filename="../src/MuscleAdapter.cpp" line="301"/>
        <source>Building output</source>
        <translation>Подготовка результата</translation>
    </message>
    <message>
        <source>Profile is not aligned</source>
        <translation type="obsolete">Выравнивание не нормализовано</translation>
    </message>
    <message>
        <location filename="../src/MuscleAdapter.cpp" line="480"/>
        <source>Aligning sequence %1 of %2</source>
        <translation>Выравнивается последовательность %1 из %2</translation>
    </message>
    <message>
        <location filename="../src/MuscleAdapter.cpp" line="512"/>
        <source>Merging results: %1 of %2</source>
        <translation>Объединяются результаты: %1 из %2</translation>
    </message>
    <message>
        <location filename="../src/MuscleUtils.cpp" line="69"/>
        <source>Unsupported alphabet: %1</source>
        <translation>Неподходящий алфавит: %1 </translation>
    </message>
</context>
<context>
    <name>U2::MuscleAddSequencesToProfileTask</name>
    <message>
        <location filename="../src/MuscleTask.cpp" line="213"/>
        <source>MUSCLE align profiles &apos;%1&apos; vs &apos;%2&apos;</source>
        <translation>MUSCLE выравнивает &apos;%1&apos; к &apos;%2&apos;</translation>
    </message>
    <message>
        <location filename="../src/MuscleTask.cpp" line="215"/>
        <source>MUSCLE align &apos;%2&apos; by profile &apos;%1&apos;</source>
        <translation>MUSCLE добавляет &apos;%2&apos; к &apos;%1&apos;</translation>
    </message>
    <message>
        <source>input_format_error</source>
        <translation type="obsolete">Не удалось определить формат файла</translation>
    </message>
    <message>
        <location filename="../src/MuscleTask.cpp" line="272"/>
        <source>Sequences in file have different alphabets %1</source>
        <translation>Последовательности в файле имеют разные алфавиты: %1</translation>
    </message>
    <message>
        <location filename="../src/MuscleTask.cpp" line="292"/>
        <source>No sequences found in file %1</source>
        <translation>Файл не содержит последовательностей: %1</translation>
    </message>
    <message>
        <location filename="../src/MuscleTask.cpp" line="294"/>
        <source>No alignment found in file %1</source>
        <translation>Файл не содержит выравниваний: %1</translation>
    </message>
</context>
<context>
    <name>U2::MuscleAlignDialogController</name>
    <message>
        <location filename="../src/MuscleAlignDialogController.cpp" line="48"/>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <location filename="../src/MuscleAlignDialogController.cpp" line="48"/>
        <source>Illegal alignment region</source>
        <translation>Неправильный регион</translation>
    </message>
    <message>
        <source>Selecting machines error!</source>
        <translation type="obsolete">Ошибка выбора удаленных машин!</translation>
    </message>
    <message>
        <source>You didn&apos;t select a machine to run remote task!</source>
        <translation type="obsolete">Не выбрана машина для удаленного запуска!</translation>
    </message>
    <message>
        <source>Distributed run on many machines is not supported yet. Select 1 machine</source>
        <translation type="obsolete">Распределенный запуск на нескольких машинах в данный пока не поддерживается. Выберите одну машину</translation>
    </message>
    <message>
        <location filename="../src/MuscleAlignDialogController.cpp" line="168"/>
        <source>MUSCLE default</source>
        <translation>По умолчанию</translation>
    </message>
    <message>
        <location filename="../src/MuscleAlignDialogController.cpp" line="169"/>
        <source>&lt;p&gt;The default settings are designed to give the best accuracy</source>
        <translation>&lt;p&gt;Наилучшая точность выравнивания</translation>
    </message>
    <message>
        <location filename="../src/MuscleAlignDialogController.cpp" line="170"/>
        <source>&lt;p&gt;&lt;b&gt;Command line:&lt;/b&gt; muscle &lt;no-parameters&gt;</source>
        <translation>&lt;p&gt;&lt;b&gt;Командная строка:&lt;/b&gt; muscle &lt;no-parameters&gt;</translation>
    </message>
    <message>
        <location filename="../src/MuscleAlignDialogController.cpp" line="174"/>
        <source>Large alignment</source>
        <translation>Большие выравнивания</translation>
    </message>
    <message>
        <location filename="../src/MuscleAlignDialogController.cpp" line="175"/>
        <source>&lt;p&gt;If you have a large number of sequences (a few thousand), or they are very long, then the default settings may be too slow for practical use. A good compromise between speed and accuracy is to run just the first two iterations of the algorithm</source>
        <translation>&lt;p&gt;При наличии тысяч последовательностей либо их большой длине, конфигурация по умолчанию может оказаться неприемлемо медленной. Хороший компромисс между скоростью и точностью обеспечивается при прогоне только первых 2-х итераций алгоритма. </translation>
    </message>
    <message>
        <location filename="../src/MuscleAlignDialogController.cpp" line="176"/>
        <source>&lt;p&gt;&lt;b&gt;Command line:&lt;/b&gt; muscle &lt;i&gt;-maxiters 2&lt;/i&gt;</source>
        <translation>&lt;p&gt;&lt;b&gt;Командная строка:&lt;/b&gt; muscle &lt;i&gt;-maxiters 2&lt;/i&gt;</translation>
    </message>
    <message>
        <location filename="../src/MuscleAlignDialogController.cpp" line="180"/>
        <source>Refine only</source>
        <translation>Только улучшить</translation>
    </message>
    <message>
        <location filename="../src/MuscleAlignDialogController.cpp" line="181"/>
        <source>&lt;p&gt;Improves existing alignment without complete realignment</source>
        <translation>&lt;p&gt;Улучшение существующего выравнивания</translation>
    </message>
    <message>
        <location filename="../src/MuscleAlignDialogController.cpp" line="182"/>
        <source>&lt;p&gt;&lt;b&gt;Command line:&lt;/b&gt; muscle &lt;i&gt;-refine&lt;/i&gt;</source>
        <translation>&lt;p&gt;&lt;b&gt;Командная строка:&lt;/b&gt; muscle &lt;i&gt;-refine&lt;/i&gt;</translation>
    </message>
</context>
<context>
    <name>U2::MuscleAlignWithExtFileSpecifyDialogController</name>
    <message>
        <location filename="../src/MuscleAlignDialogController.cpp" line="94"/>
        <source>Select input file</source>
        <translation type="unfinished">Выберите входной файл</translation>
    </message>
    <message>
        <location filename="../src/MuscleAlignDialogController.cpp" line="129"/>
        <source>Error</source>
        <translation type="unfinished">Ошибка</translation>
    </message>
    <message>
        <location filename="../src/MuscleAlignDialogController.cpp" line="129"/>
        <source>Illegal alignment region</source>
        <translation type="unfinished">Неправильный регион</translation>
    </message>
</context>
<context>
    <name>U2::MuscleGObjectTask</name>
    <message>
        <location filename="../src/MuscleTask.cpp" line="320"/>
        <source>MUSCLE align &apos;%1&apos;</source>
        <translation>MUSCLE выравнивает &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../src/MuscleTask.cpp" line="323"/>
        <source>MUSCLE refine &apos;%1&apos;</source>
        <translation>MUSCLE улучшает &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../src/MuscleTask.cpp" line="326"/>
        <source>MUSCLE add to profile &apos;%1&apos;</source>
        <translation>MUSCLE добавляет в выравнивание &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../src/MuscleTask.cpp" line="329"/>
        <source>MUSCLE align profiles</source>
        <translation>MUSCLE выравнивает пару выравниваний</translation>
    </message>
    <message>
        <location filename="../src/MuscleTask.cpp" line="344"/>
        <source>object_removed</source>
        <translation>Объект был удалён</translation>
    </message>
    <message>
        <location filename="../src/MuscleTask.cpp" line="348"/>
        <location filename="../src/MuscleTask.cpp" line="371"/>
        <source>object_is_state_locked</source>
        <translation>Объект заблокирован</translation>
    </message>
</context>
<context>
    <name>U2::MuscleLocalTask</name>
    <message>
        <source>Muscle local task</source>
        <translation type="obsolete">Внешняя задача выравнивания Muscle</translation>
    </message>
    <message>
        <source>No settings given</source>
        <translation type="obsolete">Не заданы настройки задачи</translation>
    </message>
    <message>
        <source>Muscle local task. Alignment: %1</source>
        <translation type="obsolete">Внешняя задача выравнивания Muscle. Множественное выравнивание:&apos;%1&apos;</translation>
    </message>
    <message>
        <source>Alien Muscle task from remote machine.</source>
        <translation type="obsolete">Внешняя задача выравнивания Muscle с удаленной машины.</translation>
    </message>
    <message>
        <source>Alignment</source>
        <translation type="obsolete">Множественное выравнивание</translation>
    </message>
    <message>
        <source>Task finished with error</source>
        <translation type="obsolete">Задача завершилась с ошибкой</translation>
    </message>
</context>
<context>
    <name>U2::MuscleMSAEditorContext</name>
    <message>
        <location filename="../src/MusclePlugin.cpp" line="122"/>
        <source>Align sequences to profile with MUSCLE</source>
        <translation>Выровнять с дополнительными последовательностями ...</translation>
    </message>
    <message>
        <location filename="../src/MusclePlugin.cpp" line="129"/>
        <source>Align profile to profile with MUSCLE</source>
        <translation>Выровнять с другим выравниванием ...</translation>
    </message>
    <message>
        <location filename="../src/MusclePlugin.cpp" line="192"/>
        <source>Select file with sequences</source>
        <translation>Выбор файла последовательностей</translation>
    </message>
    <message>
        <location filename="../src/MusclePlugin.cpp" line="209"/>
        <source>Select file with alignment</source>
        <translation>Выбор файла выравнивания</translation>
    </message>
    <message>
        <location filename="../src/MusclePlugin.cpp" line="115"/>
        <source>Align with MUSCLE</source>
        <translation>Выровнять ...</translation>
    </message>
</context>
<context>
    <name>U2::MuscleParallelTask</name>
    <message>
        <location filename="../src/MuscleParallel.cpp" line="28"/>
        <source>MuscleParallelTask</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>U2::MusclePlugin</name>
    <message>
        <location filename="../src/MusclePlugin.cpp" line="36"/>
        <location filename="../src/MusclePlugin.cpp" line="45"/>
        <source>MUSCLE</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/MusclePlugin.cpp" line="37"/>
        <source>A port of MUSCLE package for multiple sequence alignment. Check http://www.drive5.com/muscle/ for the original version</source>
        <translation>Порт пакета MUSCLE для выравнивания множественных последовательностей. ￼Сайт оригинального пакета http://www.drive5.com/muscle/</translation>
    </message>
    <message>
        <location filename="../src/MusclePlugin.cpp" line="51"/>
        <source>Multiple alignment</source>
        <translation type="unfinished">Множественное выравнивание</translation>
    </message>
    <message>
        <source>Multiple Alignment</source>
        <translation type="obsolete">Множественное Выравнивание</translation>
    </message>
</context>
<context>
    <name>U2::MusclePrepareTask</name>
    <message>
        <location filename="../src/MuscleParallel.cpp" line="72"/>
        <source>Internal parallel MUSCLE error: %1</source>
        <translation>Внутренняя ошибка: %1</translation>
    </message>
    <message>
        <location filename="../src/MuscleParallel.cpp" line="112"/>
        <source>No sequences in input file</source>
        <translation>Выравнивание не содержит данных</translation>
    </message>
    <message>
        <location filename="../src/MuscleParallel.cpp" line="139"/>
        <source>alignment_is_empty</source>
        <translation>Выравнивание не содержит данных</translation>
    </message>
</context>
<context>
    <name>U2::MuscleRemoteToGobjectTask</name>
    <message>
        <source>Malignment object removed</source>
        <translation type="obsolete">Объект множественного выравнивания удален</translation>
    </message>
    <message>
        <source>remote machine settings</source>
        <translation type="obsolete">настройки удаленной машины</translation>
    </message>
    <message>
        <source>MAlignment object is state locked</source>
        <translation type="obsolete">объект множественного выравнивания используется другой задачей</translation>
    </message>
    <message>
        <source>Cannot create remote machine</source>
        <translation type="obsolete">Не удалось создать удаленную машину</translation>
    </message>
    <message>
        <source>remote task didn&apos;t produced result</source>
        <translation type="obsolete">Удаленная задача не вернула результат</translation>
    </message>
    <message>
        <source>Muscle task runned on remote machine.</source>
        <translation type="obsolete">Задача Muscle, исполняющаяся на удаленной машине.</translation>
    </message>
    <message>
        <source>Alignment</source>
        <translation type="obsolete">Множественное выравнивание</translation>
    </message>
    <message>
        <source>Task finished with error</source>
        <translation type="obsolete">Задача завершилась с ошибкой</translation>
    </message>
</context>
<context>
    <name>U2::MuscleSchemaTask</name>
    <message>
        <location filename="../src/MuscleSchemaTask.cpp" line="15"/>
        <source>MuscleSchemaTask</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/MuscleSchemaTask.cpp" line="50"/>
        <source>Unable to create temporary files</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>U2::MuscleTask</name>
    <message>
        <location filename="../src/MuscleTask.cpp" line="40"/>
        <source>MUSCLE alignment</source>
        <translation>Выравнивание MUSCLE</translation>
    </message>
</context>
<context>
    <name>U2::MuscleWithExtFileSpecifySupportTask</name>
    <message>
        <location filename="../src/MuscleTask.cpp" line="407"/>
        <source>input_format_error</source>
        <translation type="unfinished">Не удалось определить формат файла</translation>
    </message>
</context>
<context>
    <name>U2::ProgressiveAlignTask</name>
    <message>
        <location filename="../src/MuscleParallel.cpp" line="251"/>
        <source>ProgressiveAlignTask</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/MuscleParallel.cpp" line="273"/>
        <source>Internal parallel MUSCLE error: %1</source>
        <translation>Внутренняя ошибка: %1</translation>
    </message>
    <message>
        <location filename="../src/MuscleParallel.cpp" line="278"/>
        <source>alignment &quot;%1&quot; Parallel MUSCLE Iter 1 accomplished. Time elapsed %2 ms</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>U2::ProgressiveAlignWorker</name>
    <message>
        <location filename="../src/MuscleParallel.cpp" line="329"/>
        <source>ProgressiveAlignWorker</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/MuscleParallel.cpp" line="343"/>
        <source>Internal parallel MUSCLE error: %1</source>
        <translation>Внутренняя ошибка: %1</translation>
    </message>
</context>
<context>
    <name>U2::RefineTask</name>
    <message>
        <location filename="../src/MuscleParallel.cpp" line="503"/>
        <source>RefineTask</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/MuscleParallel.cpp" line="526"/>
        <source>Internal parallel MUSCLE error: %1</source>
        <translation>Внутренняя ошибка: %1</translation>
    </message>
</context>
<context>
    <name>U2::RefineTreeTask</name>
    <message>
        <location filename="../src/MuscleParallel.cpp" line="458"/>
        <source>RefineTreeTask</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/MuscleParallel.cpp" line="470"/>
        <source>Internal parallel MUSCLE error: %1</source>
        <translation>Внутренняя ошибка: %1</translation>
    </message>
</context>
<context>
    <name>U2::RefineWorker</name>
    <message>
        <location filename="../src/MuscleParallel.cpp" line="579"/>
        <source>Internal parallel MUSCLE error: %1</source>
        <translation type="unfinished">Внутренняя ошибка: %1</translation>
    </message>
</context>
</TS>
