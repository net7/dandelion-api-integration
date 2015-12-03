<?php

class DataTxtApiDB {

  protected $table_vocabulary;
  protected $table_term;
  protected $isClassification = FALSE;
  protected $isEntityExtraction = FALSE;
  protected $isUserInterests = FALSE;
  protected $enabledVocabularyVariable;
  protected $modelUpdateTimeVariable;
  protected $minScoreVariable;
  protected $minScoreMultiplier;
  protected $termMatchUser = FALSE;
  protected static $instance = NULL;

  /**
   * Constructor
   * 
   * Defines attributes for this class
   *
   */
  public function __construct() {
    
  }

  /**
   * Gets the instance of current class if already created or instantiates it
   *
   * @return the instance of current class.
   */
  public static function getInstance() {
    if (empty(self::$instance)) {
      self::$instance = new DataTxtApiDB();
    }
    return self::$instance;
  }

  /**
   * getMinScoreVar
   *
   * @return the required min score float value as shown by Drupal to users
   */
  public function getMinScoreVar() {
    return variable_get($this->minScoreVariable, 0.5);
  }

  /**
   * getAPIMinScoreVar
   * 
   * In order to show to users coherent and understandable score values where:
   * 
   * 0.0 = select everything
   * 0.5 = select about the half of the results
   * 1.0 = filter out everything 
   * 
   * the min score shown by the application is obtained re-calculating 
   * the actual score ranges required by the different services.
   * 
   * The multiplier that is applied to each service is set in defines.inc.
   * and can be tuned there.
   * 
   * @return the real min score float value that is used for calls to service
   * @see: See Also datatxt_api_integration.defines
   */
  public function getAPIMinScoreVar() {
    $min_score = $this->getMinScoreVar();
    return $this->getAPIMinScoreValue($min_score);
  }

  /**
   * getMinScoreValue
   * 
   * @param: $score
   * the API real min score
   * 
   * @return the balanced score as stored in Drupal
   * @see: See Also datatxt_api_integration.defines
   */
  public function getMinScoreValue($score = NULL) {
    if (empty($score)) {
      return $this->log('DataTXT get score: undefined score');
    }
    $min_score = $score / $this->minScoreMultiplier;
    return ($min_score > 1) ? 1 : $min_score;
  }

  /**
   * getAPIMinScoreValue
   * 
   * @param: $score
   * the balanced score as stored in Drupal
   * 
   * @return the API real min score
   * @see: See Also datatxt_api_integration.defines
   */
  public function getAPIMinScoreValue($score = NULL) {
    if (empty($score)) {
      return $this->log('DataTXT get api score: undefined score');
    }
    $min_score = $score * $this->minScoreMultiplier;
    return ($min_score > 1) ? 1 : $min_score;
  }

  /**
   * Loads a DataTXT vocabulary
   *
   * @param $vid
   *   The $table_vocabulary vid value, foreign-key to taxonomy_vocabulary vid
   * @param $langcode
   *   The $table_vocabulary language value or NULL to select all languages
   *
   * @return the record(s) in $table_term with requested vid and requested langcode
   */
  public function loadVocabulary($vid, $langcode = NULL) {
    if (empty($vid)) {
      return $this->log('DataTXT vocabulary load: undefined vid');
    }
    $query = db_select($this->table_vocabulary, 't')
        ->fields('t')
        ->condition('vid', $vid, '=');
    if (DataTxtApiUtil::isValidLangcode($langcode)) {
      $query->condition('language', $langcode, '=');
    }
    $vocabulary = $query->execute()->fetchObject();
    return $vocabulary;
  }

  /**
   * Saves a DataTXT vocabulary
   *
   * @param $vid
   *   The $table_vocabulary vid value, foreign-key to taxonomy_vocabulary vid
   * @param $datatxt_id
   *   1) NULL when a new vocabulary is assigned to a DataTXT type and stored in 
   *      $enabledVocabularyVariable
   *   2) The unique key returned by DataTXT when a model is created or updated
   * @param $langcode
   *   NULL when a new vocabulary is assigned to a DataTXT type
   *   The language returned by DataTXT when a model is created or updated

   * @return the result of db operation or FALSE if something went wrong
   * 
   * @see: See Also 
   *   1) datatxt_api_integration.admin
   *   2) datatxt_api_integration > taxonomy_form_vocabulary_alter hook.
   * 
   */
  public function saveVocabulary($vid, $datatxt_id = NULL, $langcode = NULL) {
    if (empty($vid)) {
      return $this->log('DataTXT vocabulary save: undefined vid');
    }
    $fields = array();
    $fields['vid'] = $vid;
    $fields['modified'] = REQUEST_TIME;
    if (isset($datatxt_id)) {
      $fields['datatxt_id'] = $datatxt_id;
    }
    if (isset($langcode)) {
      $fields['language'] = $langcode;
      db_query("DELETE FROM " . $this->table_vocabulary . " WHERE vid = '$vid' AND language IS NULL")
          ->execute();
    }

    $vocabulary_old = db_select($this->table_vocabulary, 't')
            ->fields('t', array('did'))
            ->condition('vid', $vid, '=')
            ->condition('language', $langcode, '=')
            ->execute()->fetchObject();

    if (empty($vocabulary_old)) {
      $result = db_insert($this->table_vocabulary)
          ->fields($fields)
          ->execute();
    }
    else {
      $result = db_update($this->table_vocabulary)
          ->fields($fields)
          ->condition('vid', $vid, '=')
          ->condition('language', $langcode, '=')
          ->execute();
    }
    return (!empty($result));
  }

  /**
   * Deletes a DataTXT vocabulary
   * 
   * This does NOT delete a taxonomy_vocabulary but the records in DataTXT 
   * $table_vocabulary that map that taxonomy_vocabulary item with a DataTXT 
   * stored model.
   *
   * @param $vid
   *   The $table_vocabulary vid value, foreign-key to taxonomy_vocabulary vid
   * @param $datatxt_id
   *   1) NULL when a new vocabulary is assigned to a DataTXT type and stored in 
   *      $enabledVocabularyVariable
   *   2) The unique key returned by DataTXT when a model is created or updated
   * @param $langcode
   *   The $table_vocabulary language value or NULL to select all languages

   * @return the result of db operation or FALSE if something went wrong
   * 
   * @see: See Also datatxt_api_integration > taxonomy_vocabulary_delete hook.
   * 
   */
  public function deleteVocabulary($vid, $langcode = NULL) {
    if (empty($vid)) {
      return $this->log('DataTXT vocabulary delete: undefined vid');
    }
    if (!empty($langcode) && $this->isDataTxt($vid, $langcode)) {
      $vocabulary = $this->loadVocabulary($vid, $langcode);
      if ($this->isClassification) {
        $datatxt_id = $vocabulary->datatxt_id;
        $dataTxtClassificationService = DataTxtClassificationService::getInstance();
        $result = $dataTxtClassificationService->sendClassificationModelToDataTxt(NULL, 'delete', $datatxt_id);
      }
    }
    $this->deleteVocabularyTopics($vid, $langcode);
    $query = db_delete($this->table_vocabulary)
        ->condition('vid', $vid);
    if (!empty($langcode)) {
      $query->condition('language', $langcode);
    }
    $result = $query->execute();
    $vid = $this->getEnabledVocabularyVid();
    if (!empty($vid) && !$this->isDataTxt($vid)) {
      variable_set($this->enabledVocabularyVariable, '');
    }
    return $result;
  }

  /**
   * Detects or updates info about changed status and time of change of enabled vocabulary.
   * This is useful to avoid sending the model to DataTXT when not necessary and 
   * not before a certain time defined in settings.
   * 
   *
   * @param $langcode
   *   The language of the class enabled vocabulary
   * @param $new_status
   *   The new status if boolean, or NULL to retrieve current status.
   * @param $new_time
   *   The time of change.
   *
   * @return boolean: TRUE if changed, FALSE if unchanged;
   * 
   */
  protected function vocabularyChanged($langcode, $new_status = NULL, $new_time = FALSE) {
    $status = variable_get($this->enabledVocabularyVariable . '_changed', NULL);
    if (!isset($status)) {
      $status = array($langcode => array('changed' => TRUE, 'time' => time()));
      variable_set($this->enabledVocabularyVariable . '_changed', $status);
    }
    if (!isset($status[$langcode])) {
      $status[$langcode] = array('changed' => TRUE, 'time' => time());
      variable_set($this->enabledVocabularyVariable . '_changed', $status);
    }
    if (isset($new_status) && $new_status != $status[$langcode]['changed']) {
      $status[$langcode]['changed'] = $new_status;
      variable_set($this->enabledVocabularyVariable . '_changed', $status);
    }
    if ($new_time && $new_time != $status[$langcode]['time']) {
      $status[$langcode]['time'] = $new_time;
      variable_set($this->enabledVocabularyVariable . '_changed', $status);
    }
    return $status[$langcode];
  }

  /**
   * Selects all topics stored in datatxt table_term related to a taxonomy term.
   *
   * @param int $tid
   *   The taxonomy_term_data tid
   * @param string $langcode
   *   The term language
   *
   * @return object[]
   *   An associative array with all selected datatxt terms.
   */
  public function loadTopics($tid, $langcode) {
    if (empty($tid)) {
      return $this->log('DataTXT topics load: undefined tid');
    }
    if (empty($langcode)) {
      return $this->log('DataTXT topics load: undefined language');
    }
    $result = db_select($this->table_term, 't')
        ->fields('t')
        ->condition('tid', $tid, '=')
        ->condition('language', $langcode, '=')
        ->orderBy((!$this->isEntityExtraction) ? 'score' : 'category', 'DESC')
        ->execute()
        ->fetchAll();
    return $result;
  }

  /**
   * Preparare a renderable array of the provided topics.
   *
   * @param object[] $topics
   *   The topics.
   *
   * @return array
   *   The renderable array.
   */
  public function viewTopics($topics) {
    $list = array();

    foreach ($topics as $topic) {
      $list[] = $this->filterWikipediaURL($topic->topic);
    }

    return array(
      '#theme' => 'item_list',
      '#items' => $list,
      '#attributes' => array(
        'class' => array('datatxt-user-topics'),
      ),
    );
  }

  /**
   * Saves a single topic in datatxt table_term.
   *
   * @param array $fields
   *   The fields associative array required by insert or update query
   *
   * @return DatabaseStatementInterface|int
   *   The DB operation result.
   */
  public function saveTopic($fields) {
    if (empty($fields)) {
      return $this->log('DataTXT topic save: no data');
    }
    if (!$this->isEntityExtraction && empty($fields['score'])) {
      return $this->log('DataTXT topic save: missing score');
    }
    if (empty($fields['tid']) || empty($fields['vid']) || empty($fields['topic'])) {
      return $this->log('DataTXT topic save: missing fields');
    }
    $fields['modified'] = REQUEST_TIME;

    $cond = db_and();
    $cond->condition('topic', $fields['topic'], '=')
        ->condition('language', $fields['language'], '=')
        ->condition('tid', $fields['tid'], '=');

    $exists = $query = db_select($this->table_term, 'd')
        ->fields('d')
        ->condition($cond)
        ->execute()
        ->fetch();

    if (empty($exists)) {
      $result = db_insert($this->table_term)->fields($fields)->execute();
    }
    else {
      $result = db_update($this->table_term)->fields($fields)->condition($cond)->execute();
    }
    $this->vocabularyChanged($fields['language'], TRUE);
    return $result;
  }

  /**
   * Selects all taxonomy terms inclusive of datatxt topics
   * of a given taxonomy vocabulary.
   * Also performs a check to detect and remove orphan topics, 
   * operation is logged.
   *
   * @param int $vid
   *   The taxonomy_vocabulary vid
   * @param string $langcode
   *   The term language
   *
   * @return array
   *   An associative array with all selected extended taxonomy terms.
   */
  public function loadVocabularyTerms($vid, $langcode) {
    if (empty($vid)) {
      return $this->log('Table taxonomy_term_data select: undefined vid');
    }
    if (empty($langcode)) {
      return $this->log('Table taxonomy_term_data select: undefined language');
    }
    $data = array();
    $categories = db_select($this->table_term, 't')
        ->fields('t', array('tid'))
        ->condition('vid', $vid, '=')
        ->condition('language', $langcode, '=')
        ->groupBy('tid')
        ->execute()
        ->fetchCol('tid');
    if (!empty($categories)) {
      foreach ($categories as $tid) {
        $terms = taxonomy_term_load($tid);
        if (!empty($terms)) {
          $desc = DataTxtApiUtil::getTranslatedTermName($tid, $langcode);
          $topics = db_select($this->table_term, 't')
              ->fields('t', array('topic', 'score'))
              ->condition('t.tid', $tid, '=')
              ->condition('t.language', $langcode, '=')
              ->execute()
              ->fetchAll();
          if (!empty($topics)) {
            $data[$desc] = array();
            foreach ($topics as $topic) {
              $dataDesc = array();
              $dataDesc ['topic'] = $topic->topic;
              if (!$this->isEntityExtraction) {
                $dataDesc['score'] = $topic->score;
              }
              $data[$desc][] = $dataDesc;
            }
          }
        }
        else {
          $this->log('Found and deleted orphan DataTXT topics associated to missing term tid: %tid, language: %language', array(
            '%tid' => $tid,
            '%language' => $langcode
              ), 'warning', TRUE, TRUE);
          $this->deleteTopics($tid, $langcode);
        }
      }
    }
    return $data;
  }

  /**
   * Deletes all topics stored in datatxt table_term related to a taxonomy term
   *
   * @param $tid
   *   The taxonomy_term_data tid
   * @param $langcode
   *   The term language, if missing all topics for the given tid are deleted
   *
   * @return The DB operation result.
   * 
   */
  public function deleteTopics($tid, $langcode = NULL) {
    if (empty($tid)) {
      return $this->log('DataTXT topics delete: undefined tid');
    }
    $query = db_delete($this->table_term)
        ->condition('tid', $tid, '=');
    if (!empty($langcode)) {
      $query->condition('language', $langcode, '=');
    }

    $result = $query->execute();
    if ($result) {
      $this->vocabularyChanged($langcode, TRUE);
    }
    return $result;
  }

  /**
   * Deletes all topics stored in datatxt table_term related to a datatxt vocabulary
   *
   * @param $vid
   *   The taxonomy_vocabulary tid associated to the datatxt vocabulary
   * @param $langcode
   *   The term language, if missing all topics for the given vid are deleted
   *
   * @return The DB operation result.
   * 
   */
  public function deleteVocabularyTopics($vid, $langcode = NULL) {
    if (empty($vid)) {
      return $this->log('DataTXT vocabulary terms delete: undefined vid');
    }
    $query = db_delete($this->table_term)
        ->condition('vid', $vid, '=');
    if (!empty($langcode)) {
      $query->condition('language', $langcode, '=');
    }
    $result = $query->execute();
    return $result;
  }

  /**
   * Detects if a vocabulary is enabled for some datatxt features, 
   * depending on subclass.
   *
   * @param $vid
   *   The taxonomy_vocabulary tid associated to the datatxt vocabulary
   * @param $langcode
   *   The vocabulary language, if missing any language is enough
   *
   * @return boolean
   * 
   */
  public function isDataTxt($vid, $langcode = NULL) {
    if (!empty($vid)) {
      $query = db_select($this->table_vocabulary, 't')
          ->fields('t', array('vid'))
          ->condition('vid', $vid, '=');
      if (!empty($langcode)) {
        $query->condition('language', $langcode, '=');
      }
      $result = $query->execute()->fetchCol();
      return empty($result) ? FALSE : $result;
    }
    return FALSE;
  }

  /**
   * Detects if the requested note_type has some associated datatxt topics.
   *
   * @param $node_type
   *
   * @return boolean
   * @see datatxt_api_integration_form_alter
   */
  public function hasDataTxtTaxonomy($node_type) {
    $bundle_instances = field_info_instances('node', $node_type);
    $vocabularies = array();
    foreach ($bundle_instances as $name => $instance) {
      $field_info = field_info_field($name);
      if ("taxonomy_term_reference" === $field_info['type']) {
        $vocabulary_name = $field_info['settings']['allowed_values']['0']['vocabulary'];
        $voc = taxonomy_vocabulary_machine_name_load($vocabulary_name);
        if ($this->isDataTxt($voc->vid)) {
          $vocabularies[$name] = $voc;
        }
      }
    }
    return empty($vocabularies) ? FALSE : $vocabularies;
  }

  /**
   * Detects if the requested taxonomy_vocabulary has some datatxt topics.
   * Useful to display the "Send to DataTXT" button when there 
   * actually is some data to send. 
   *
   * @param $vid
   * The taxonomy vocabulary vid
   *
   * @return boolean
   * @see datatxt_api_integration_form_taxonomy_form_vocabulary_alter
   */
  public function hasVocabularyTerms($vid) {
    if (empty($vid)) {
      return $this->log('Table taxonomy_term_data select: undefined vid');
    }
    $result = db_select('taxonomy_term_data', 't')
        ->fields('t', array('tid'))
        ->condition('vid', $vid, '=')
        ->execute()
        ->fetchAssoc();
    return ($result !== FALSE);
  }

  /**
   * Retrieves the taxonomy_vocabulary_name filtering by vocabulary vid 
   * and language, if not NULL
   * 
   * @param $vid
   * @param $langcode
   */
  public function getVocabularyName($vid, $langcode = NULL) {
    if (empty($vid)) {
      return $this->log('Table taxonomy_vocabulary select: undefined vid');
    }
    $taxonomy_vocabulary = taxonomy_vocabulary_load($vid);
    if (!empty($taxonomy_vocabulary)) {
      if (!empty($langcode) && module_exists('i18n')) {
        $name = i18n_taxonomy_vocabulary_name($taxonomy_vocabulary, $langcode);
      }
      else {
        $name = $taxonomy_vocabulary->name;
      }
      return $name;
    }
    return FALSE;
  }

  /**
   * Retrieves the variable setting that defines the enabled vocabulary for this instance.
   */
  public function getEnabledVocabularyVid() {
    return variable_get($this->enabledVocabularyVariable, NULL);
  }

  /**
   * Retrieves the variable setting that defines model synchronization delay time.
   */
  public function getModelUpdateTime() {
    return variable_get($this->modelUpdateTimeVariable, 0);
  }

  /**
   * Gets enabled dataTXT vocabulary for this instance.
   *
   * @param $form_state
   *   A keyed array containing the current state of the form.
   * 
   * @return: vocabulary vid or FALSE
   * 
   */
  public function getEnabledVocabulary($langcode = NULL) {
    $vid = $this->getEnabledVocabularyVid();
    if (!empty($vid)) {
      return $this->loadVocabulary($vid, $langcode);
    }
    return FALSE;
  }

  /**
   * Gets language
   *
   * @param $form_state
   *   A keyed array containing the current state of the form.
   * 
   * @return: langcode
   * 
   */
  public function getLanguage(&$form_state = NULL) {
    global $language;

    if (!empty($form['#term']) && DataTxtApiUtil::isValidLangcode($form['#term']->language)) {
      return $form['#term']->language;
    }
    if (!empty($form['#vocabulary']) && DataTxtApiUtil::isValidLangcode($form['#vocabulary']->language)) {
      return $form['#vocabulary']->language;
    }
    if (empty($form_state)) {
      return $language->language;
    }
    if (!empty($form_state['storage']) && !empty($form_state['storage']['entity_translation'])) {
      return $form_state['storage']['entity_translation']['handler']['taxonomy_term']->getFormLanguage();
    }
    if (!empty($form_state['values']) && !empty($form_state['values']['language']) && DataTxtApiUtil::isValidLangcode($form_state['values']['language'])) {
      return $form_state['values']['language'];
    }
    return $language->language;
  }

  /**
   * Gets taxonomy term tid
   *
   * @param $term_name
   * @param $langcode
   * 
   * @return: tid
   * 
   */
  public function getTermIdByNameAndLanguage($term_name, $langcode) {
    error_log(html_entity_decode($term_name, ENT_QUOTES));
    $result = db_select('field_data_name_field', 't')
        ->fields('t', array('entity_id'))
        ->condition('name_field_value', html_entity_decode($term_name, ENT_QUOTES), '=')
        ->condition('language', $langcode, '=')
        ->execute()
        ->fetchAll();
    return $result;
  }

  /**
   * Add WikiSearch required css and js to page
   * 
   */
  public static function addWikiSearchHead() {
    drupal_add_library('system', 'ui.effects.fade');
    drupal_add_css(drupal_get_path('module', 'datatxt_api_integration') . '/css/admin.css');
    drupal_add_js(array('datatxt' => array('title_new' => t('New'))), 'setting');
    drupal_add_js(drupal_get_path('module', 'datatxt_api_integration') . '/js/admin.js');
  }

  /**
   * Adds WikiSearch fields to the form passed by reference
   *
   * @param $form
   *   Nested array of form elements that comprise the form.
   * @param $form_state
   *   A keyed array containing the current state of the form.
   * 
   */
  public function addWikiSearch(&$form, &$form_state) {

    global $language;

    // Options
    $defaultOptions = array(
      //  wikisearch box title
      'title' => 'Wikisearch',
      // field weight in form element
      'weight' => 50,
      // max number of topics
      'maxTopics' => 15,
      // expose wiki url and score fields
      'fullMode' => FALSE,
      // default score for topics, final score when not fullMode
      'defaultScore' => 0.5,
      // is wikisearch box collapsible 
      'outerCollapsible' => FALSE,
      // is wikisearch box collapsed at start
      'outerCollapsed' => FALSE,
      // are wikisearch topics collapsible 
      'innerCollapsible' => TRUE,
      // are wikisearch topics collapsed at start
      'innerCollapsed' => TRUE,
      // use when you need a different language than page language, example: user profile
      'language' => $language->language
    );

    if (!empty($form_state['values']['datatxt_langcode'])) {
      $options['language'] = $form_state['values']['datatxt_langcode'];
    }
    if (empty($form_state['values']['wikisearch_options'])) {
      $options = $defaultOptions;
    }
    else {
      $options = array();
      $customOptions = $form_state['values']['wikisearch_options'];
      foreach ($defaultOptions as $k => $v) {
        $options[$k] = (!empty($customOptions[$k])) ? $customOptions[$k] : $defaultOptions[$k];
      }
      if (!$options['outerCollapsible']) {
        $options['outerCollapsed'] = FALSE;
      }
      if (!$options['innerCollapsible']) {
        $options['innerCollapsed'] = FALSE;
      }
    }

    self::addWikiSearchHead();

    // Add fields for datatxt topic.
    $form['datatxt'] = array(
      '#type' => 'fieldset',
      '#title' => t($options['title']),
      '#collapsible' => $options['outerCollapsible'],
      '#collapsed' => $options['outerCollapsed'],
      '#weight' => $options['weight'],
      '#prefix' => '<div class="datatxt-term-admin">',
      '#suffix' => '</div>',
    );

    $form['class_name'] = array('#type' => 'hidden', '#default_value' => get_class($this));
    $form['datatxt_langcode'] = array('#type' => 'hidden', '#default_value' => $options['language']);
    $form['datatxt_maxitems'] = array('#type' => 'hidden', '#value' => $options['maxTopics']);

    $form['datatxt']['topics'] = array(
      '#type' => 'container',
      '#weight' => -50,
      '#attributes' => array(
        'id' => 'datatxt-topics'
      )
    );

    $topics = array();
    if (!empty($form_state['term']->tid)) {
      $tid = $form_state['term']->tid;
      $topics = $this->loadTopics($tid, $options['language']);
    }
    elseif ($this->termMatchUser && !empty($form_state['user']->name)) {
      $tid = $this->getTidByUserName($form_state['user']->name);
      if (!empty($tid)) {
        $topics = $this->loadTopics($tid, $options['language']);
      }
    }

    $std = new stdClass();
    $std->topic = '';
    $std->score = $options['defaultScore'];
    $count_avail_topics = $options['maxTopics'] - count($topics);

    for ($i = 0; $i < $count_avail_topics; $i++) {
      array_push($topics, $std);
    }

    $score_options = array();

    for ($i = 0; $i <= 100; $i++) {
      $val = strval($i / 100);
      $score_options[$val] = $val;
    }

    for ($i = 0; $i < count($topics); $i++) {

      $topic = $topics[$i];
      $topic->topic = (!empty($topic->topic)) ? $topic->topic : '';
      $topic->score = (!empty($topic->score)) ? $topic->score : $options['defaultScore'];

      $form['datatxt']['topics']['topic_' . $i] = array(
        '#type' => 'fieldset',
        '#collapsible' => $options['innerCollapsible'],
        '#collapsed' => $options['innerCollapsed'],
        '#weight' => $i - 50,
      );
      if (empty($topic->topic)) {
        $form['datatxt']['topics']['topic_' . $i]['#title'] = '<span class="var">' . t('New') . '</span>';
        $form['datatxt']['topics']['topic_' . $i]['#prefix'] = '<div class="topic">';
        $form['datatxt']['topics']['topic_' . $i]['#suffix'] = '</div>';
      }
      else {
        // Filter the topic name.
        $topic_title = $this->filterWikipediaURL($topic->topic);

        $form['datatxt']['topics']['topic_' . $i]['#title'] = '<span class="var">' . $topic_title . '</span>';
        $form['datatxt']['topics']['topic_' . $i]['#prefix'] = '<div class="topic active">';
        $form['datatxt']['topics']['topic_' . $i]['#suffix'] = '</div>';
      }
      $form['datatxt']['topics']['topic_' . $i]['#title'] .= '<span class="action-delete btn btn-mini">' . t('Delete') . '</span>';
      if (!empty($options['fullMode'])) {
        $form['datatxt']['topics']['topic_' . $i]['url_' . $i] = array(
          '#type' => 'textfield',
          '#title' => t('Datasource URL'),
          '#description' => t('URL for DataTXT taxonomy'),
          '#maxlength' => 255,
          '#default_value' => $topic->topic,
          '#attributes' => array(
            'id' => 'edit-url-' . $i,
            'class' => array(
              'url-item'
            )
          )
        );
        $form['datatxt']['topics']['topic_' . $i]['score_' . $i] = array(
          '#type' => 'select',
          '#title' => t('Score'),
          '#description' => t('Relevance of topic'),
          '#options' => $score_options,
          '#default_value' => $topic->score,
          '#attributes' => array(
            'id' => 'edit-topic-' . $i,
            'class' => array(
              'score-item'
            )
          )
        );
      }
      else {
        $form['datatxt']['topics']['topic_' . $i]['url_' . $i] = array(
          '#type' => 'hidden',
          '#default_value' => $topic->topic,
          '#attributes' => array(
            'id' => 'edit-url-' . $i,
          )
        );
        $form['datatxt']['topics']['topic_' . $i]['score_' . $i] = array(
          '#type' => 'hidden',
          '#value' => $topic->score,
          '#attributes' => array(
            'id' => 'edit-topic-' . $i,
          )
        );
      }
      $form['datatxt']['topics']['topic_' . $i][] = array(
        '#type' => 'button',
        '#name' => 'datatxt_topic_search_button_' . $i,
        '#value' => t('Find related terms'),
        '#prefix' => '<div class="form-item form-item-search-btn"><label>&nbsp;</label>',
        '#suffix' => '</div>',
        '#limit_validation_errors' => array(),
        '#ajax' => array(
          'callback' => $this->wikiSearchCallback,
          'progress' => array(
            'type' => 'throbber',
            'message' => NULL,
          )
        ),
        '#attributes' => array(
          'class' => array(
            'btn-mini'
          )
        )
      );
      $form['datatxt']['topics']['topic_' . $i]['search_txt_' . $i] = array(
        '#type' => 'textfield',
        '#title' => t('Search terms'),
        '#description' => t('Input topic description'),
        '#maxlength' => 255,
        '#prefix' => '<div class="form-item form-item-search-txt">',
        '#suffix' => '</div>',
      );
      $form['datatxt']['topics']['topic_' . $i][] = array(
        '#markup' => '<div id="datatxt_api_integration_term_search_results_' . $i . '" '
        . 'class="form-item-result"><div></div></div>',
      );
      $form['datatxt']['topics']['topic_' . $i][] = array(
        '#markup' => '<div id="datatxt_api_integration_term_search_results_js_' . $i . '"></div>',
      );
    }
    $form['datatxt']['add-action'] = array(
      '#markup' => '<a class="add' . (($count_avail_topics < 1) ? ' hidden' : '') . '">' . t('Add a new Wikipedia entry') . '</a>',
    );
  }

  /**
   * WikiSearch callback
   * 
   * @param type $form
   * @param type $form_state
   */
  public function wikiSearchCallback(&$form, &$form_state) {
    if (preg_match("/(\d+)$/", $form_state['triggering_element']['#name'], $matches)) {
      $group_index = $matches[0];
      $wrapperRef = '#datatxt_api_integration_term_search_results_' . $group_index . ' > div';
      $wrapperJSRef = '#datatxt_api_integration_term_search_results_js_' . $group_index;
      $search_txt = $form_state['values']['search_txt_' . $group_index];
      if (empty($search_txt)) {
        $commands = array(ajax_command_replace($wrapperRef, t('Please, insert text')));
        return array('#type' => 'ajax', '#commands' => $commands);
      }
      $langcode = $form_state['values']['datatxt_langcode'];
      $commands = array();
      $results = array();
      $service = DataTxtClassificationService::getInstance();
      $response = $service->datatxtWikiSearch($search_txt, $langcode);
      // if not service error
      if (!empty($response)) {
        foreach ($response['entities'] as $entity) {
          $results[] = array(
            '#theme' => 'datatxt_wikisearch_item',
            '#entity' => $entity,
          );
        }
      }
      if (!empty($results)) {
        $form['results'] = $results;
        $js = '<script stye="text/javascript">jQuery.fn.topics(' . $group_index . ')</script>';
        $commands[] = ajax_command_replace($wrapperRef, drupal_render($results));
        $commands[] = ajax_command_replace($wrapperJSRef, $js);
        return array('#type' => 'ajax', '#commands' => $commands);
      }
      else {
        return FALSE;
      }
    }
    watchdog('ERROR', t('Missing index in function ') . 'datatxt_api_integration_term_search_callback', WATCHDOG_ERROR);
    return FALSE;
  }

  /**
   * Inserts or updates a modified enabled taxonomy_vocabulary to DataTXT service
   *
   * @vid The taxonomy_vocabulary primary key
   * @langcode The langcode
   * @debug writes to watchdog even if datatxt_debug_mode is Off: default FALSE
   * @display display on screen: default FALSE
   * @force forces the send of modified vocabulary regardless of scheduled delays
   * 
   * @return: FALSE
   * 
   */
  public function saveModelToDataTxt($vid, $langcode, $debug = FALSE, $display = FALSE, $force = FALSE) {
    if (empty($vid)) {
      return $this->log('saveModelToDataTxt select: undefined vid', array(), 'error', $debug, $display);
    }
    if (!DataTxtApiUtil::isValidLangcode($langcode)) {
      return $this->log('saveModelToDataTxt select: undefined language', array(), 'error', $debug, $display);
    }
    if (!$this->isDataTxt($vid, $langcode)) {
      return FALSE;
    }
    $time = time();
    $ev = $this->getModelUpdateTime();
    if (!$force && empty($ev)) {
      return FALSE;
    }
    $status = $this->vocabularyChanged($langcode);
    if (!$status['changed']) {
      return $this->log(t('Data (%lan) was not sent as vocabulary has not changed.', array(
            '%lan' => $langcode)), array(), 'info', $debug, $display);
    }
    else if (!$force && $time - $status['time'] < $ev) {
      return $this->log(t('Data (%lan) was not sent because scheduled at a later time.', array(
            '%lan' => $langcode)), array(), 'info', $debug, $display);
    }

    $dataTxtClassificationService = DataTxtClassificationService::getInstance();
    $vcdata = $this->prepareModelToDataTxt($vid, $langcode);
    if (!empty($vcdata)) {
      // drupal_set_message(drupal_json_encode($vcdata));
      $model = $vcdata->model;
      $vocabulary = $vcdata->vocabulary;
      if (empty($vocabulary) || empty($vocabulary->datatxt_id)) {
        $datatxt_id = $dataTxtClassificationService->sendClassificationModelToDataTxt($model, 'create', NULL);
        if (!empty($datatxt_id)) {
          if ($this->saveVocabulary($vid, $datatxt_id, $langcode)) {
            $this->log(t('DataTXT (%lan) created sucessfully', array(
                  '%lan' => $langcode)) . ': Model ID ' . $datatxt_id, array(), 'info', $debug, $display);
            $this->vocabularyChanged($langcode, FALSE, $time);
          }
          else {
            $this->log(t('An error occured with the DataTXT call'), array(), 'error', $debug, $display);
          }
        }
      }
      else {
        $datatxt_id = $vocabulary->datatxt_id;
        $result = $dataTxtClassificationService->sendClassificationModelToDataTxt($model, 'update', $datatxt_id);
        if (!empty($result)) {
          $this->log(t('DataTXT (%lan) updated sucessfully', array(
                '%lan' => $langcode)) . ': Model ID ' . $datatxt_id, array(), 'info', $debug, $display);
          $this->vocabularyChanged($langcode, FALSE, $time);
        }
      }
    }
  }

  /**
   * Structures all terms and topics info as required by DataTXT indexing service.
   *
   * @vid The taxonomy_vocabulary primary key
   * @langcode The langcode
   * 
   * @return: structured data or FALSE if something went wrong.
   * 
   */
  protected function prepareModelToDataTxt($vid, $langcode) {
    if (empty($vid)) {
      return $this->log('prepareModelToDataTxt: undefined vid');
    }
    if (empty($langcode)) {
      return $this->log('prepareModelToDataTxt: undefined language');
    }
    $data = new stdClass();
    $vocabularyTerms = $this->loadVocabularyTerms($vid, $langcode);
    if (!empty($vocabularyTerms)) {
      $model = new stdClass();
      $model->lang = $langcode;
      $model->description = $this->getVocabularyName($vid, $langcode);
      $model->categories = array();
      foreach ($vocabularyTerms as $categoryName => $categoryData) {
        $category = new stdClass();
        $category->name = $categoryName;
        $category->topics = new stdClass();
        foreach ($categoryData as $categoryItem) {
          $topic = $categoryItem['topic'];
          $category->topics->$topic = floatval($categoryItem['score']);
        }
        $model->categories[] = $category;
      }
      $data->model = $model;
      $data->vocabulary = $this->loadVocabulary($vid, $langcode);
      return $data;
    }
    return FALSE;
  }

  /**
   * Sends models for supoported languages to DataTXT
   *
   * @params $isCron
   * specifies different behaviour if manually send or in cron context.
   * 
   * @see datatxt_api_integration_cron.
   * 
   */
  public function sendModelsToDataTxt($isCron = FALSE) {
    $model_update_time = $this->getModelUpdateTime();
    if (!$isCron || !empty($model_update_time)) {
      $vid = $this->getEnabledVocabularyVid();
      $available_languages = array_keys(language_list('language'));
      foreach ($available_languages as $langcode) {
        $this->saveModelToDataTxt($vid, $langcode, FALSE, !$isCron, !$isCron);
      }
    }
  }

  /**
   * Extract concepts
   *
   * Overriden in subclasses
   *
   * @param $content
   *   The string being processed
   * @param $langcode
   *
   * @return: an array of concept objects or NULL if none filtered or function
   * @see: See Also DataTxtClassificationService 
   * 
   */
  public function getConcepts($content, $langcode) {
    return FALSE;
  }

  /**
   * Publish node 
   * Use to update directly on db, without calling api, running hooks etc.
   *
   * @param $nid
   *   The node nid
   */
  public static function publish($nid) {
    db_query("UPDATE {node_revision} SET `status` = '1' WHERE `nid` =:nid ;", array(':nid' => $nid));
    db_query("UPDATE {node} SET `status` = '1' WHERE `nid` =:nid ;", array(':nid' => $nid));
    cache_clear_all(NULL, 'cache_page', '*');
  }

  /**
   * Unpublish node 
   * Use to update directly on db, without calling api, running hooks etc.
   *
   * @param $nid
   *   The node nid
   */
  public static function unpublish($nid) {
    db_query("UPDATE {node_revision} SET `status` = '0' WHERE `nid` =:nid ;", array(':nid' => $nid));
    db_query("UPDATE {node} SET `status` = '0' WHERE `nid` =:nid ;", array(':nid' => $nid));
    cache_clear_all(NULL, 'cache_page', '*');
  }

  /**
   * Logging and error reporting
   *
   * 1) Stores a message on watchdog if datatxt_debug_mode is On or if set by params
   * 2) Displays a message on screen if set by params
   *
   * @msg The message string to display
   * @watchdog_params The associative array passed as third parameter to watchdog.
   * @errorlevel info/warning/error
   * @debug forces storage to watchdog table even if datatxt_debug_mode is Off: default FALSE
   * @display display on screen: default FALSE
   * 
   * @return: FALSE
   * 
   */
  protected function log($msg, $watchdog_params = array(), $errorlevel = 'error', $debug = FALSE, $display = FALSE) {
    if ($debug || variable_get('datatxt_debug_mode', TRUE)) {
      $watchdog_levels = array(
        'info' => WATCHDOG_INFO,
        'warning' => WATCHDOG_WARNING,
        'error' => WATCHDOG_ERROR);
      watchdog('DataTXT', $msg, $watchdog_params, $watchdog_levels[$errorlevel]);
      if ($display) {
        drupal_set_message(t($msg, $watchdog_params), $errorlevel);
      }
    }
    return FALSE;
  }

  /**
   * Convert the Wikipedia URL in a nice title.
   *
   * @param string $url
   *   The Wikipedia URL.
   *
   * @return string
   *   The nice title.
   */
  protected function filterWikipediaURL($url) {
    $url = preg_replace('/http:\/\/[a-z]{2}\.wikipedia\.org\/wiki\//', '', $url);
    $url = str_replace('_', ' ', $url);
    return $url;
  }

}
