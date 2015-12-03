<?php

class DataTxtApiUserInterestsDB extends DataTxtApiDB {

  protected $table_article;

  /**
   * {@inheritdoc}
   */
  public function __construct() {
    $this->table_vocabulary = 'datatxt_user_interests_vocabulary';
    $this->table_term = 'datatxt_user_interests_term';
    $this->table_article = 'datatxt_user_articles';
    $this->isUserInterests = TRUE;
    $this->enabledVocabularyVariable = 'datatxt_user_interests_vocabulary';
    $this->modelUpdateTimeVariable = 'datatxt_user_interest_model_update';
    // Same as entity extraction as concepts with lower score would be cut off anyways
    $this->minScoreVariable = 'datatxt_entity_extraction_min_score';
    $this->minScoreMultiplier = floatval(DATATXT_CLASSIFICATION_SCORE_MULTIPLIER);
    $this->termMatchUser = TRUE;
    $this->wikiSearchCallback = 'callback_datatxt_api_integration_user_interest_wikisearch';
  }

  /**
   * {@inheritdoc}
   */
  public static function getInstance() {
    if (empty(self::$instance) || get_class(self::$instance) != 'DataTxtApiUserInterestsDB') {
      self::$instance = new DataTxtApiUserInterestsDB();
    }
    return self::$instance;
  }

  /**
   * Gets the user object for a given taxonomy_term_data tid inside the 
   * vocabulary referred in enabledVocabularyVariable.
   * Since for that vocabulary terms are referred to users the values of
   * fields taxonomy_term_data name and users name match.
   *
   * @param $tid
   *   The uid to look for
   *
   * @return the user object or FALSE if not found or something went wrong.
   */
  public function getUserByTid($tid = NULL) {
    global $user;
    if (!empty($tid)) {
      $taxonomy_term = taxonomy_term_load($tid);
      $name = $taxonomy_term->name;
      $uid = db_select('users', 'u')
          ->fields('u', array('uid'))
          ->condition('name', $name, '=')
          ->execute()
          ->fetchField();
      return (!empty($uid)) ? user_load($uid) : FALSE;
    }
    return $user;
  }

  /**
   * Gets the taxonomy_term_data tid for a given name inside the vocabulary 
   * referred in enabledVocabularyVariable.
   * Since for that vocabulary terms are referred to users the values of
   * fields taxonomy_term_data name and users name match.
   *
   * @param string $username
   *   The record fields so as required by db_insert and db_update methods
   *
   * @return object|bool
   *   The taxonomy_term_data tid named as $username or FALSE if
   *   not found or something went wrong.
   */
  public function getTidByUserName($username) {
    if (!empty($username)) {
      $tid = db_select('taxonomy_term_data', 't')
          ->fields('t', array('tid'))
          ->condition('name', $username, '=')
          ->condition('vid', $this->getEnabledVocabularyVid(), '=')
          ->execute()
          ->fetchField();
      return (!empty($tid)) ? $tid : FALSE;
    }
    return FALSE;
  }

  /**
   * Inserts or updates DataTXT records to $table_article table
   *
   * @param $fields
   *   The record fields so as required by db_insert and db_update methods
   *
   * @return the result of db operation or FALSE if something went wrong
   */
  public function saveArticle($fields) {
    if (empty($fields)) {
      return $this->log('Table datatxt_user_article save: empty data');
    }
    // All required
    foreach ($fields as $field_name => $field_data) {
      if (empty($field_data)) {
        return $this->log('DataTXT datatxt_user_article save: missing value for column ' . $field_name);
      }
    }
    $fields['modified'] = REQUEST_TIME;
    $exists = $query = db_select($this->table_article, 'a')
        ->fields('a')
        ->condition('nid', $fields['nid'], '=')
        ->condition('uid', $fields['uid'], '=')
        ->execute()
        ->fetch();
    if (empty($exists)) {
      $result = db_insert($this->table_article)
          ->fields($fields)
          ->execute();
    }
    else {
      $result = db_update($this->table_article)
          ->fields($fields)
          ->condition('topic', $fields['topic'], '=')
          ->condition('language', $fields['language'], '=')
          ->execute();
    }
    return $result;
  }

  /**
   * Deletes DataTXT records from $table_article table
   *
   * @param $field_name
   *   The name of the field to filter by.
   * @param $field_value
   *   The value of the field.
   *
   * @return the result of db operation or FALSE if something went wrong
   */
  public function deleteArticleBy($field_name, $field_value) {
    if (!empty($field_name) && !empty($field_value)) {
      $result = db_delete($this->table_article)
          ->condition($field_name, $field_value, '=')
          ->execute();

      return $result;
    }
    return FALSE;
  }

  /**
   * {@inheritdoc}
   */
  public function getConcepts($content, $langcode = LANGUAGE_NONE) {
    $vid = $this->getEnabledVocabularyVid();
    $vocabulary = $this->loadVocabulary($vid, $langcode);
    if (!empty($vocabulary)) {
      $min_score = $this->getAPIMinScoreVar();
      $service = DataTxtClassificationService::getInstance();
      $result = $service->classifyHtmlFragmentContent($content, $vocabulary->datatxt_id, $min_score);
      return(sizeof($result) > 0) ? $result : FALSE;
    }
    return FALSE;
  }

  /**
   * Creates matches users - node articles based on user-interests.
   *
   * Checks for the concepts extracted for an article, 
   * filters them by user-interests-scores and stores 
   * a match uid - nid in datatxt user interests articles table.
   *
   * @param $concept_data
   *   Concepts so as provided by classification service
   *   structured as an array of concept objects
   * @param $nid
   *   The node nid of the article being processed
   *
   * @see: See Also DataTxtClassificationService 
   *   and datatxt_api_integration_node_insert/update
   * 
   */
  public function processArticleUserInterests($concept_data, $nid) {

    if (empty($concept_data)) {
      return FALSE;
    }

    $vid = $this->getEnabledVocabularyVid();

    // Extracted data: assoc. [ concept->topic => concept ]
    $concepts = array();
    foreach ($concept_data as $concept) {
      $concepts [$concept->uri_representation] = $concept;
    }

    // Existing data: assoc. [ uid => {topic, score} ]
    $q1 = db_select($this->table_term, 't');
    $q1->fields('u', array('uid'));
    $q1->fields('t', array('topic'));
    $q1->fields('t', array('score'));
    $q1->join('taxonomy_term_data', 'd', 'd.tid = t.tid');
    $q1->join('users', 'u', 'd.name = u.name');
    $q1->condition('t.vid', $vid);
    $q1->condition('t.topic', array_keys($concepts), 'in');
    $r1 = $q1->execute();
    $term_groups = $r1->fetchAll(PDO::FETCH_GROUP);
    // Debug: dpq($query1); dpm($uid__term);
    // 
    // Existing art. matching: assoc. [ uid => did ]
    if (!empty($term_groups)) {
      $q2 = db_select($this->table_article, 't');
      $q2->fields('t', array('uid', 'did'));
      $q2->condition('t.nid', $nid);
      $q2->condition('t.uid', array_keys($term_groups), 'in');
      $r2 = $q2->execute();
      $articles = $r2->fetchAllKeyed(0);
      // Debug: dpq($q2); dpm($uid__did);
    }
    else {
      $articles = array();
    }

    $fields = array(
      'insert' => array(),
      'update' => array()
    );

    foreach ($term_groups as $uid => $term_group) {
      foreach ($term_group as $term) {
        $score_current = $this->getMinScoreValue($concepts[$term->topic]->entity_score);
        $score_required = $term->score;
        if ($score_required <= $score_current) {
          $field = array(
            'nid' => $nid,
            'uid' => $uid,
            'score' => $score_current,
            'modified' => REQUEST_TIME
          );
          if (empty($articles[$uid])) {
            $op = 'insert';
            $key = "uid{$uid}nid{$nid}";
          }
          else {
            $op = 'update';
            $key = $articles[$uid];
          }
          $is_new = !isset($fields[$op][$key]);
          $is_better_score = (isset($fields[$op][$key]) && $fields[$op][$key]['score'] < $field['score']);
          if ($is_new || $is_better_score) {
            $fields[$op][$key] = $field;
          }
        }
        else if (variable_get('datatxt_debug_mode', FALSE)) {
          watchdog('DataTXT User Interests', t('Low score for source %source and user uid %uid: required: %req, score: %cur', array(
            '%source' => $term->topic,
            '%uid' => $uid,
            '%req' => $score_required,
            '%cur' => $score_current,
          )));
        }
      }
    }
    // Debug: dpm($fields_insert); dpm($fields_update);

    if (!empty($fields['insert'])) {
      foreach ($fields['insert'] as $fi) {
        db_insert($this->table_article)->fields($fi)->execute();
      }
    }
    if (!empty($fields['update'])) {
      foreach ($fields['update'] as $did => $fu) {
        db_update($this->table_article)->fields($fu)->condition('did', $did)->execute();
      }
    }
  }

  /**
   * Selects all nodes referred to current user based on user interests.
   *
   * @param int $uid
   *   The User uid.
   *
   * @return int[]|bool
   *   The array of nid or FALSE is user is not logged in.
   */
  public function getUserInterestsArticles($uid = NULL) {
    if (!$uid) {
      global $user;
      $uid = $user->uid;
    }

    if (!empty($uid)) {
      $nids = db_select($this->table_article, 't')
          ->fields('t', array('nid'))
          ->condition('uid', $uid, '=')
          ->execute()
          ->fetchAll(PDO::FETCH_COLUMN);
      return $nids;
    }
    return FALSE;
  }

}

/**
 * Callback for WikiSearch
 *
 * Calls instance's wikiSearchCallback method
 *
 * @param &$form
 *   The form array; this is a reference so the caller can get back
 *   whatever information the form(s) involved left for it.
 * @param &$form_state
 *   The form state array; this is a reference so the caller can get back
 *   whatever information the form(s) involved left for it.
 *
 * @ingroup callbacks
 */
function callback_datatxt_api_integration_user_interest_wikisearch(&$form, &$form_state) {
  $api = DataTxtApiUserInterestsDB::getInstance();
  return $api->wikiSearchCallback($form, $form_state);
}
