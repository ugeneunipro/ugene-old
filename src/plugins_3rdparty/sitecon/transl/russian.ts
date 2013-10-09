<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="ru_RU">
<context>
    <name>SiteconBuildDialog</name>
    <message>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="13"/>
        <source>SITECON build</source>
        <translation>Построение модели SITECON</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="21"/>
        <source>Input alignment (nucleic)</source>
        <translation>Входное выравнивание</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="31"/>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="48"/>
        <source>...</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="38"/>
        <source>Output model</source>
        <translation>Результирующая модель</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="57"/>
        <source>Options</source>
        <translation>Настройки</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="66"/>
        <source>Window size</source>
        <translation>Размер окна</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="98"/>
        <source>bp</source>
        <translation>нукл</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="117"/>
        <source>Calibration random seed</source>
        <translation>Случайная затравка</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="168"/>
        <source>Calibration sequence length</source>
        <translation>Длина калибровочной последовательности</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="201"/>
        <source>       100 Kb</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="206"/>
        <source>       500 Kb</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="211"/>
        <source>         1 Mb</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="216"/>
        <source>         5 Mb</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="227"/>
        <source>Weight algorithm</source>
        <translation>Алгоритм весов</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="285"/>
        <source>Default status</source>
        <translation> </translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="305"/>
        <source>Build</source>
        <translation>Построить</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="312"/>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="254"/>
        <source>       None</source>
        <translation>Без весов</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="259"/>
        <source>  Algorithm N2</source>
        <translation>Алгоритм №2</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="63"/>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="92"/>
        <location filename="../src/SiteconBuildWorker.cpp" line="73"/>
        <source>Window is used to pick out the most important alignment region and is located at the center of the alignment.
Must be: windows size is not greater than TFBS alignment length, recommended: windows size is not greater than 50 bp.</source>
        <translation>Размер окна должен быть не короче консенсуса для данного сайта но не более длины выравнивания.</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="165"/>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="188"/>
        <location filename="../src/SiteconBuildWorker.cpp" line="74"/>
        <source>Length of random synthetic sequences used to calibrate the profile. Should not be less than window size.</source>
        <translation>Длина случайных синтетических последовательностей для калибрации профайла. Не должна быть меньше размера окна.</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="114"/>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="143"/>
        <location filename="../src/SiteconBuildWorker.cpp" line="75"/>
        <source>The random seed, where &lt;n&gt; is a positive integer. You can use this option to generate reproducible results for different runs on the same data.</source>
        <translation>Случайная затравка. Использование этого параметра полезно для воспроизведения результата калибрации.</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="224"/>
        <location filename="../src/ui/SiteconBuildDialog.ui" line="247"/>
        <location filename="../src/SiteconBuildWorker.cpp" line="76"/>
        <source>Optional feature, in most cases applying no weight will fit. In some cases choosing algorithm 2 will increase the recognition quality.</source>
        <translation>В большинстве случаев алгоритм без весов вполне адекватен. Использование других алгоритмов может улучшить качество распознавания в отдельных случаях.</translation>
    </message>
</context>
<context>
    <name>SiteconSearchDialog</name>
    <message>
        <location filename="../src/ui/SiteconSearchDialog.ui" line="14"/>
        <source>SITECON search</source>
        <translation>Поиск сайтов связывания ТФ по модели SITECON</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconSearchDialog.ui" line="22"/>
        <source>File with model</source>
        <translation>Модель</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconSearchDialog.ui" line="36"/>
        <source>...</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconSearchDialog.ui" line="46"/>
        <source>Threshold</source>
        <translation>Минимальная ошибка</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconSearchDialog.ui" line="76"/>
        <source>Strands</source>
        <translation>Направление</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconSearchDialog.ui" line="82"/>
        <source>Both strands</source>
        <translation>Все</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconSearchDialog.ui" line="95"/>
        <source>Search in direct strand only</source>
        <translation>Искать ССТФ только в прямой последовательности</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconSearchDialog.ui" line="98"/>
        <source>Direct strand</source>
        <translation>Только прямые</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconSearchDialog.ui" line="114"/>
        <source>Search in complement strand</source>
        <translation>Искать ССТФ только в комплeментарной последовательности</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconSearchDialog.ui" line="117"/>
        <source>Complement strand</source>
        <translation>Только комплeментарные</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconSearchDialog.ui" line="142"/>
        <source>Range</source>
        <translation>Регион</translation>
    </message>
    <message>
        <source>Whole sequence</source>
        <translation>Вся последовательность</translation>
    </message>
    <message>
        <source>Selection range</source>
        <translation>Селектированный регион</translation>
    </message>
    <message>
        <source>Custom range</source>
        <translation>Указанный регион</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconSearchDialog.ui" line="147"/>
        <source>Strand</source>
        <translation>Стренд</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconSearchDialog.ui" line="152"/>
        <source>PSUM</source>
        <translation>Оценка</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconSearchDialog.ui" line="157"/>
        <source>First type error</source>
        <translation>Ошибка 1-го рода</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconSearchDialog.ui" line="162"/>
        <source>Second type error</source>
        <translation>Ошибка 2-го рода</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconSearchDialog.ui" line="172"/>
        <source>Clear results</source>
        <translation>Сбросить результаты</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconSearchDialog.ui" line="179"/>
        <source>Save as annotations</source>
        <translation>Сохранить результаты</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconSearchDialog.ui" line="203"/>
        <source>Ready</source>
        <translation>Готово</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconSearchDialog.ui" line="223"/>
        <source>Search</source>
        <translation>Искать</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconSearchDialog.ui" line="230"/>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <location filename="../src/ui/SiteconSearchDialog.ui" line="43"/>
        <location filename="../src/ui/SiteconSearchDialog.ui" line="59"/>
        <location filename="../src/SiteconQuery.cpp" line="167"/>
        <location filename="../src/SiteconSearchWorker.cpp" line="84"/>
        <source>Recognition quality threshold, should be less than 100%. Choosing too low threshold will lead to recognition of too many TFBS recognised with too low trustworthiness. Choosing too high threshold may result in no TFBS recognised.</source>
        <translation>Нижний предел качества распознавания, величина более 60% но меньше 100%. Слишком низкий предел приведёт к нахождению большого числа ложных сигналов, чрезмерно высокий отбросит качественные результаты.</translation>
    </message>
</context>
<context>
    <name>U2::GTest_CompareSiteconModels</name>
    <message>
        <location filename="../src/SiteconAlgorithmTests.cpp" line="586"/>
        <source>Models not equal</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::SiteconBuildPrompter</name>
    <message>
        <location filename="../src/SiteconBuildWorker.cpp" line="122"/>
        <source>For each MSA from &lt;u&gt;%1&lt;/u&gt;,</source>
        <translation>Для каждого выравнивания из &lt;u&gt;%1&lt;/u&gt;,</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildWorker.cpp" line="123"/>
        <source>%1 build SITECON model.</source>
        <translation>%1 построить профайл SITECON.</translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::SiteconBuildWorker</name>
    <message>
        <location filename="../src/SiteconBuildWorker.cpp" line="84"/>
        <source>Build SITECON Model</source>
        <translation>Построить SITECON</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildWorker.cpp" line="143"/>
        <source>Random seed can not be less zero</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildWorker.cpp" line="147"/>
        <source>Calibration length can not be less zero</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildWorker.cpp" line="152"/>
        <source>Window size can not be less zero</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildWorker.cpp" line="174"/>
        <source>Built SITECON model from: %1</source>
        <translation>Построен профайл SITECON из %1</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildWorker.cpp" line="60"/>
        <source>Input alignment</source>
        <translation>Входное выравнивание</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildWorker.cpp" line="66"/>
        <source>Sitecon model</source>
        <translation>Профайл SITECON</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildWorker.cpp" line="61"/>
        <source>Origin</source>
        <translation>Происхождение</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildWorker.cpp" line="73"/>
        <source>Window size, bp</source>
        <translation>Размер окна</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildWorker.cpp" line="74"/>
        <source>Calibration length</source>
        <translation>Длина калибрации</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildWorker.cpp" line="75"/>
        <source>Random seed</source>
        <translation>Затравка</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildWorker.cpp" line="76"/>
        <source>Weight algorithm</source>
        <translation>Алгоритм весов</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildWorker.cpp" line="60"/>
        <source>Input multiple sequence alignment for building statistical model.</source>
        <translation>Входное множественное выравнивание для построения статистической модели данного ССТФ.</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildWorker.cpp" line="61"/>
        <source>Location of input alignment, used as optional hint for model description.</source>
        <translation>Адрес множественного выравнивания, используется для описания выходного профайла SITECON.</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildWorker.cpp" line="66"/>
        <source>Produced statistical model of specified TFBS data.</source>
        <translation>Произведённый профайл SITECON.</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildWorker.cpp" line="85"/>
        <source>Builds statistical profile for SITECON. The SITECON is a program for probabilistic recognition of transcription factor binding sites.</source>
        <translation>Строит статистический профайл для поиска ССТФ при помощи пакета SITECON.</translation>
    </message>
    <message>
        <source>None</source>
        <translation type="obsolete">Без весов</translation>
    </message>
    <message>
        <source>Algorithm2</source>
        <translation type="obsolete">Алгоритм №2</translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::SiteconReadPrompter</name>
    <message>
        <location filename="../src/SiteconIOWorkers.cpp" line="188"/>
        <source>Read model(s) from &lt;u&gt;%1&lt;/u&gt;.</source>
        <translation>Загрузить профайлы SITECON из &lt;u&gt;%1&lt;/u&gt;.</translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::SiteconReader</name>
    <message>
        <location filename="../src/SiteconIOWorkers.cpp" line="228"/>
        <source>Loaded SITECON model from %1</source>
        <translation>Загружен профайл SITECON из %1</translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::SiteconSearchPrompter</name>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="147"/>
        <source>unset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="148"/>
        <source>For each sequence from &lt;u&gt;%1&lt;/u&gt;,</source>
        <translation>Для каждой последовательности из &lt;u&gt;%1&lt;/u&gt;,</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="162"/>
        <source>%1 search transcription factor binding sites (TFBS) %2.&lt;br&gt;Recognize sites with &lt;u&gt;similarity %3%&lt;/u&gt;, process &lt;u&gt;%4&lt;/u&gt;.&lt;br&gt;Output the list of found regions annotated as &lt;u&gt;%5&lt;/u&gt;.</source>
        <translation>%1 искать сайты связывания ТФ %2.&lt;br&gt;Распознавать сайты с достоверностью &lt;u&gt;не менее %3%&lt;/u&gt;, рассматривать &lt;u&gt;%4 стренды&lt;/u&gt;.&lt;br&gt;Выдать список найденных регионов аннотированных как &lt;u&gt;%5&lt;/u&gt;.</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="149"/>
        <source>with all profiles provided by &lt;u&gt;%1&lt;/u&gt;,</source>
        <translation>со всеми профайлами из &lt;u&gt;%1&lt;/u&gt;,</translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::SiteconSearchWorker</name>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="156"/>
        <source>both strands</source>
        <translation>все</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="157"/>
        <source>direct strand</source>
        <translation>только прямые</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="158"/>
        <source>complement strand</source>
        <translation>только комплиментарные</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="65"/>
        <source>Sitecon Model</source>
        <translation>Профайл SITECON</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="66"/>
        <source>Sequence</source>
        <translation>Входная последовательность</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="67"/>
        <source>SITECON annotations</source>
        <translation>Аннотированные ССТФ</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="81"/>
        <source>Result annotation</source>
        <translation>Аннотации</translation>
    </message>
    <message>
        <source>Search in</source>
        <translation type="obsolete">Направление</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="98"/>
        <source>Search for TFBS with SITECON</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="194"/>
        <source>Min score can not be less 60% or more 100%</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="197"/>
        <source>Min Err1 can not be less 0 or more 1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="200"/>
        <source>Max Err2 can not be less 0 or more 1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="203"/>
        <source>Search in strand can only be 0(both) or 1(direct) or -1(complement)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="233"/>
        <source>Search TFBS in %1</source>
        <translation>Поиск ССТФ в %1</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="237"/>
        <source>Bad sequence supplied to SiteconSearch: %1</source>
        <translation>Неправильная последовательность для SITECON поиска : %1</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="261"/>
        <source>Found %1 TFBS</source>
        <translation>Найдено %1 возможных сайтов связывания ТФ</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="65"/>
        <source>Profile data to search with.</source>
        <translation>Профайл SITECON характеризующий искомые сайты.</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="66"/>
        <source>Input nucleotide sequence to search in.</source>
        <translation>Входная нуклеотидная последовательность для поиска.</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="68"/>
        <source>Annotations marking found TFBS sites.</source>
        <translation>Аннотации маркирующие найденные регионы.</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="82"/>
        <source>Annotation name for marking found regions.</source>
        <translation>Имя аннотации для разметки найденных регионов.</translation>
    </message>
    <message>
        <source>Which strands should be searched: direct, complement or both.</source>
        <translation type="obsolete">В каких стрендах последовательности следует искать ССТФ: прямых, комплементарных или всех.</translation>
    </message>
    <message>
        <source>Search TFBS</source>
        <translation type="obsolete">Поиск ССТФ</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="99"/>
        <source>Searches each input sequence for transcription factor binding sites significantly similar to specified SITECON profiles. In case several profiles were supplied, searches with all profiles one by one and outputs merged set of annotations for each sequence.</source>
        <translation>Поиск сайтов связывания транскрипционных факторов (ССТФ). В каждой нуклеотидной последовательности, поданной на вход задачи, ищутся значимые совпадения с указанными профайлами SITECON. Найденные регионы выдаются в виде набора аннотаций. Профайлов может быть несколько, поиск отработает для каждого профайла и выдаст общий набор аннотаций для каждой последовательности.&lt;p&gt; Протеиновые последовательности на входе допустимы но игнорируются.</translation>
    </message>
    <message>
        <source>Minimal threshold</source>
        <translation type="obsolete">Достоверность</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="83"/>
        <source>Min score</source>
        <translation>Мин оценка</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="85"/>
        <source>Min Err1</source>
        <translation>Мин ошибка 1 рода</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="86"/>
        <source>Alternative setting for filtering results, minimal value of Error type I.&lt;br&gt;Note that all thresholds (by score, by err1 and by err2) are applied when filtering results.</source>
        <translation>Фильтрация результатов по значению ошибки I рода. &lt;br&gt;Учтите что применяются все фильтры (по оценке и ошибкам 1, 2 рода).</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="88"/>
        <source>Max Err2</source>
        <translation>Макс ошибка 2 рода</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchWorker.cpp" line="89"/>
        <source>Alternative setting for filtering results, max value of Error type II.&lt;br&gt;Note that all thresholds (by score, by err1 and by err2) are applied when filtering results.</source>
        <translation>Фильтрация результатов по значению ошибки II рода. &lt;br&gt;Учтите что применяются все фильтры (по оценке и ошибкам 1, 2 рода).</translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::SiteconWritePrompter</name>
    <message>
        <location filename="../src/SiteconIOWorkers.cpp" line="194"/>
        <source>unset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconIOWorkers.cpp" line="197"/>
        <source>Save the profile(s) from &lt;u&gt;%1&lt;/u&gt; to %2.</source>
        <translation>Сохранить все профайлы из &lt;u&gt;%1&lt;/u&gt; в %2.</translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::SiteconWriter</name>
    <message>
        <location filename="../src/SiteconIOWorkers.cpp" line="248"/>
        <source>Unspecified URL for writing Sitecon</source>
        <translation>Не укзазан адрес для записи модели SITECON.</translation>
    </message>
    <message>
        <location filename="../src/SiteconIOWorkers.cpp" line="263"/>
        <source>Writing SITECON model to %1</source>
        <translation>Запись профайла SITECON в %1</translation>
    </message>
</context>
<context>
    <name>U2::QDSiteconActor</name>
    <message>
        <location filename="../src/SiteconQuery.cpp" line="67"/>
        <source>%1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconQuery.cpp" line="71"/>
        <source>with profile provided by %1 %2&lt;/a&gt;</source>
        <translation>с профайлом %1 %2&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../src/SiteconQuery.cpp" line="74"/>
        <source>with all %1 %2 profiles&lt;/a&gt;</source>
        <translation>со всеми %1 %2 профайлами&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../src/SiteconQuery.cpp" line="77"/>
        <source>similarity %1%</source>
        <translation>%1%</translation>
    </message>
    <message>
        <location filename="../src/SiteconQuery.cpp" line="82"/>
        <source>both strands</source>
        <translation>оба стренда</translation>
    </message>
    <message>
        <location filename="../src/SiteconQuery.cpp" line="83"/>
        <source>direct strand</source>
        <translation>прямой стренд</translation>
    </message>
    <message>
        <location filename="../src/SiteconQuery.cpp" line="84"/>
        <source>complement strand</source>
        <translation>комплементарный стренд</translation>
    </message>
    <message>
        <location filename="../src/SiteconQuery.cpp" line="109"/>
        <source>%1: incorrect sitecon model url(s)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconQuery.cpp" line="113"/>
        <source>%1: min score can not be less 60% or more 100%</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconQuery.cpp" line="117"/>
        <source>%1: min Err1 can not be less 0 or more 1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconQuery.cpp" line="121"/>
        <source>%1: max Err2 can not be less 0 or more 1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Searches transcription factor binding sites (TFBS) %1.&lt;br&gt;Recognize sites with %2, process %3.&lt;br&gt;Annotate found regions as %4.</source>
        <translation type="obsolete">Ищет сайты связывания транскрипционных факторов (ТФ) %1. &lt;br&gt;Распознаёт сайты со сходством %2, рассматривает %3. &lt;br&gt;Аннотирует найденные регионы как %4.</translation>
    </message>
    <message>
        <location filename="../src/SiteconQuery.cpp" line="161"/>
        <source>Sitecon</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconQuery.cpp" line="162"/>
        <source>Searches for transcription factor binding sites significantly similar to specified SITECON profiles. In case several profiles were supplied, searches with all profiles one by one and outputs merged set of annotations.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconQuery.cpp" line="87"/>
        <source>Searches transcription factor binding sites (TFBS) %1.&lt;br&gt;Recognize sites with %2, process %3.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Search in</source>
        <translation type="obsolete">Направление</translation>
    </message>
    <message>
        <location filename="../src/SiteconQuery.cpp" line="166"/>
        <source>Min score</source>
        <translation type="unfinished">Мин оценка</translation>
    </message>
    <message>
        <location filename="../src/SiteconQuery.cpp" line="168"/>
        <source>Min Err1</source>
        <translation type="unfinished">Мин ошибка 1 рода</translation>
    </message>
    <message>
        <location filename="../src/SiteconQuery.cpp" line="169"/>
        <source>Alternative setting for filtering results, minimal value of Error type I.&lt;br&gt;Note that all thresholds (by score, by err1 and by err2) are applied when filtering results.</source>
        <translation type="unfinished">Фильтрация результатов по значению ошибки I рода. &lt;br&gt;Учтите что применяются все фильтры (по оценке и ошибкам 1, 2 рода).</translation>
    </message>
    <message>
        <location filename="../src/SiteconQuery.cpp" line="171"/>
        <source>Max Err2</source>
        <translation type="unfinished">Макс ошибка 2 рода</translation>
    </message>
    <message>
        <location filename="../src/SiteconQuery.cpp" line="172"/>
        <source>Alternative setting for filtering results, max value of Error type II.&lt;br&gt;Note that all thresholds (by score, by err1 and by err2) are applied when filtering results.</source>
        <translation type="unfinished">Фильтрация результатов по значению ошибки II рода. &lt;br&gt;Учтите что применяются все фильтры (по оценке и ошибкам 1, 2 рода).</translation>
    </message>
    <message>
        <location filename="../src/SiteconQuery.cpp" line="174"/>
        <source>Model</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconQuery.cpp" line="174"/>
        <source>Profile data to search with.</source>
        <translation type="unfinished">Профайл SITECON характеризующий искомые сайты.</translation>
    </message>
</context>
<context>
    <name>U2::QDSiteconTask</name>
    <message>
        <location filename="../src/SiteconQuery.cpp" line="206"/>
        <source>Sitecon Query</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>U2::SiteconADVContext</name>
    <message>
        <location filename="../src/SiteconPlugin.cpp" line="123"/>
        <source>Search TFBS with SITECON...</source>
        <translation type="unfinished">SITECON: Искать сайты связывания ТФ...</translation>
    </message>
</context>
<context>
    <name>U2::SiteconBuildDialogController</name>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="68"/>
        <source>select_file_with_alignment</source>
        <translation>Выбор выравнивания для построения модели SITECON</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="104"/>
        <source>illegal_in_file_name</source>
        <translation>Укажите входной файл выравнивания</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="109"/>
        <source>illegal_out_file_name</source>
        <translation>Укажите файл результирующей модели</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="115"/>
        <source>error</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="127"/>
        <source>starting_calibration_process</source>
        <translation>Начинается построение</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="130"/>
        <source>hide_button</source>
        <translation>Скрыть</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="131"/>
        <source>cancel_button</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="144"/>
        <source>build_finished_with_errors_%1</source>
        <translation>Построение завершено с ошибкой: %1</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="146"/>
        <source>build_canceled</source>
        <translation>Построение отменено</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="148"/>
        <source>build_finished_successfuly</source>
        <translation>Построение успешно завершено</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="150"/>
        <source>start_button</source>
        <translation>Построить</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="151"/>
        <source>close_button</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="157"/>
        <source>running_state_%1_progress_%2%</source>
        <translation>Прогресс %2%: %1</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="78"/>
        <source>Select file to save model to...</source>
        <translation>Выбор файла для сохранения профайла</translation>
    </message>
</context>
<context>
    <name>U2::SiteconBuildTask</name>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="172"/>
        <source>build_sitecon_model</source>
        <translation>Построение модели SITECON</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="182"/>
        <source>alignment_has_gaps</source>
        <translation>Выравнивание содержит пропуски</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="186"/>
        <source>alignment_is_empty</source>
        <translation>Выравнивание не содержит данных</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="190"/>
        <source>Alignment must have at least 2 sequences</source>
        <translation>Выравнивание должно содержать минимум 2 последовательности</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="194"/>
        <source>alignment_is_not_nucleic</source>
        <translation>Поддерживаются только нуклеотидные выравнивания</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="198"/>
        <source>window_greater_then_length</source>
        <translation>Размер окна больше длины последовательности</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="212"/>
        <source>calculating_ave_disp_matrix</source>
        <translation>расчёт матрицы средних и дисперсии</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="217"/>
        <source>calculating_weights</source>
        <translation>расчёт весов</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="223"/>
        <source>calculating_firstTypeErr</source>
        <translation>расчёт ошибки ошибки 1-го рода</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="229"/>
        <source>calculating_second_type_err</source>
        <translation>расчёт ошибки ошибки 2-го рода</translation>
    </message>
</context>
<context>
    <name>U2::SiteconBuildToFileTask</name>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="237"/>
        <source>build_sitecon_model</source>
        <translation>Построение модели SITECON</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="247"/>
        <source>input_format_error</source>
        <translation>Неверный формат файла</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="255"/>
        <source>loading_ali</source>
        <translation>загрузка выравнивания</translation>
    </message>
    <message>
        <location filename="../src/SiteconBuildDialogController.cpp" line="274"/>
        <source>no_alignments_found</source>
        <translation>Выравнивание не найдено</translation>
    </message>
</context>
<context>
    <name>U2::SiteconIO</name>
    <message>
        <location filename="../src/SiteconIO.cpp" line="110"/>
        <source>Not a sitecon model</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/SiteconIO.cpp" line="149"/>
        <source>Error parsing settings, line %1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/SiteconIO.cpp" line="160"/>
        <source>Error parsing window size: %1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/SiteconIO.cpp" line="173"/>
        <source>Error parsing calibration len: %1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/SiteconIO.cpp" line="179"/>
        <source>Error parsing RSEED: %1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/SiteconIO.cpp" line="185"/>
        <location filename="../src/SiteconIO.cpp" line="191"/>
        <source>Error parsing number of sequence in original alignment: %1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/SiteconIO.cpp" line="199"/>
        <source>Illegal weight algorithm: %1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/SiteconIO.cpp" line="211"/>
        <source>Model size not matched: %1, expected: %2</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/SiteconIO.cpp" line="244"/>
        <source>Property not recognized: %1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/SiteconIO.cpp" line="260"/>
        <source>error_parsing_matrix_val_%1_in_line_%2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconIO.cpp" line="278"/>
        <source>error_parsing_errors_line_%1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconIO.cpp" line="287"/>
        <location filename="../src/SiteconIO.cpp" line="292"/>
        <source>error_parsing_error_val_%1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconIO.cpp" line="296"/>
        <source>illegal_err_val_%1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconIO.cpp" line="306"/>
        <source>error_parsing_file_line_%1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconIO.cpp" line="315"/>
        <source>ave_props_in_file_%1_not_matched_sdev_props_%2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconIO.cpp" line="320"/>
        <source>ave_props_in_file_%1_not_matched_weight_props_%2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconIO.cpp" line="325"/>
        <source>props_in_file_%1_not_matched_actual_props_%2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconIO.cpp" line="353"/>
        <source>error_info_not_complete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconIO.cpp" line="360"/>
        <source>model_verification_error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconIO.cpp" line="234"/>
        <source>duplicate_prop_%1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/SiteconIOWorkers.cpp" line="62"/>
        <location filename="../src/SiteconIOWorkers.cpp" line="68"/>
        <location filename="../src/SiteconIOWorkers.cpp" line="138"/>
        <location filename="../src/SiteconIOWorkers.cpp" line="147"/>
        <source>Sitecon model</source>
        <translation>Профайл SITECON</translation>
    </message>
    <message>
        <source>Location</source>
        <translation type="obsolete">Адрес</translation>
    </message>
    <message>
        <location filename="../src/SiteconIOWorkers.cpp" line="70"/>
        <source>SITECON</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/SiteconIO.cpp" line="53"/>
        <source>Sitecon models</source>
        <translation>Профайлы SITECON</translation>
    </message>
    <message>
        <source>Location of the data file(s).</source>
        <translation type="obsolete">Адрес файла данных</translation>
    </message>
    <message>
        <source>Location hint for the target file.</source>
        <translation type="obsolete">Адрес целевого файла</translation>
    </message>
    <message>
        <location filename="../src/SiteconIOWorkers.cpp" line="138"/>
        <source>Input Sitecon model</source>
        <translation>Входящий профайл SITECON</translation>
    </message>
    <message>
        <location filename="../src/SiteconIOWorkers.cpp" line="140"/>
        <source>Saves all input SITECON profiles to specified location.</source>
        <translation>Сохраняет все входящие профайлы SITECON в указанный файл.</translation>
    </message>
    <message>
        <location filename="../src/SiteconIOWorkers.cpp" line="140"/>
        <source>Write SITECON Model</source>
        <translation>Сохранение модели SITECON</translation>
    </message>
    <message>
        <location filename="../src/SiteconIOWorkers.cpp" line="147"/>
        <source>Loaded SITECON profile data.</source>
        <translation>Загруженный профайл SITECON</translation>
    </message>
    <message>
        <location filename="../src/SiteconIOWorkers.cpp" line="153"/>
        <source>Reads SITECON profiles from file(s). The files can be local or Internet URLs.</source>
        <translation>Считывает профайл(ы) SITECON из указанных файлов. Файлы могут быть локальными либо адресами Интернет.</translation>
    </message>
    <message>
        <location filename="../src/SiteconIOWorkers.cpp" line="153"/>
        <source>Read SITECON Model</source>
        <translation>Загрузка модели SITECON</translation>
    </message>
</context>
<context>
    <name>U2::SiteconPlugin</name>
    <message>
        <location filename="../src/SiteconPlugin.cpp" line="60"/>
        <source>sitecon_plugin</source>
        <translation>SITECON</translation>
    </message>
    <message>
        <location filename="../src/SiteconPlugin.cpp" line="60"/>
        <source>sitecon_plugin_desc</source>
        <translation>Адаптированная версия программы SITECON: поиск сайтов связывания транскрипционных факторов. Включает около сотни откалиброванных моделей ССТФ. 
Домашняя страница проекта: http://wwwmgs.bionet.nsc.ru/cgi-bin/mgs/sitecon/sitecon.pl?stage=0</translation>
    </message>
    <message>
        <location filename="../src/SiteconPlugin.cpp" line="66"/>
        <source>sitecon_build</source>
        <translation>Построить модель SITECON</translation>
    </message>
    <message>
        <location filename="../src/SiteconPlugin.cpp" line="70"/>
        <source>sitecon_menu</source>
        <translation>SITECON (Сайты Связывания ТФ)</translation>
    </message>
</context>
<context>
    <name>U2::SiteconReadMultiTask</name>
    <message>
        <location filename="../src/SiteconQuery.cpp" line="233"/>
        <source>Load sitecon models task</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>U2::SiteconReadTask</name>
    <message>
        <location filename="../src/SiteconIO.h" line="52"/>
        <source>Read SITECON Model</source>
        <translation>Загрузка модели SITECON</translation>
    </message>
</context>
<context>
    <name>U2::SiteconSearchDialogController</name>
    <message>
        <location filename="../src/SiteconSearchDialogController.cpp" line="137"/>
        <source>cancel_button</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchDialogController.cpp" line="137"/>
        <source>close_button</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchDialogController.cpp" line="149"/>
        <source>progress_%1%_</source>
        <translation>Прогресс  %1%</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchDialogController.cpp" line="151"/>
        <source>%1_results_found.</source>
        <translation>найдено %1</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchDialogController.cpp" line="170"/>
        <source>select_file_with_model</source>
        <translation>Выбор модели SITECON</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchDialogController.cpp" line="179"/>
        <location filename="../src/SiteconSearchDialogController.cpp" line="278"/>
        <location filename="../src/SiteconSearchDialogController.cpp" line="287"/>
        <location filename="../src/SiteconSearchDialogController.cpp" line="297"/>
        <source>error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchDialogController.cpp" line="215"/>
        <source>%1%	err1=%2	err2=%3</source>
        <translation>%1%	err1=%2	err2=%3</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchDialogController.cpp" line="278"/>
        <source>model not selected</source>
        <translation>Укажите файл модели</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchDialogController.cpp" line="287"/>
        <source>range_is_too_small</source>
        <translation>Слишком маленький регион для поиска</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchDialogController.cpp" line="297"/>
        <source>error_parsing_min_score</source>
        <translation>Неразборчивая строка минимальной ошибки</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchDialogController.cpp" line="362"/>
        <source>complement_strand</source>
        <translation>комплeментарный</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchDialogController.cpp" line="362"/>
        <source>direct_strand</source>
        <translation>прямой</translation>
    </message>
</context>
<context>
    <name>U2::SiteconSearchTask</name>
    <message>
        <location filename="../src/SiteconSearchTask.cpp" line="28"/>
        <source>sitecon_search</source>
        <translation>Поиск ССТФ</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchTask.cpp" line="43"/>
        <source>sitecon_search_parallel</source>
        <translation>Параллельный поиск ССТФ</translation>
    </message>
    <message>
        <location filename="../src/SiteconSearchTask.cpp" line="63"/>
        <source>internal_error_invalid_psum:%1</source>
        <translation>Внутренняя ошибка, неверная сумма: %1</translation>
    </message>
</context>
<context>
    <name>U2::SiteconWriteTask</name>
    <message>
        <location filename="../src/SiteconIO.h" line="65"/>
        <source>Save SITECON model</source>
        <translation>Сохранение модели SITECON</translation>
    </message>
</context>
</TS>
