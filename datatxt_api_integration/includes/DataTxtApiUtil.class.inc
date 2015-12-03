<?php

/**
 * Created by PhpStorm.
 * User: max2thousand
 * Date: 27/03/15
 * Time: 09:53
 */
class DataTxtApiUtil {

  /**
   * Tries to reconciliate entity extraction entities with classification entities.
   * In other words, it tries to count how much extracted entities match the classification ones grouping by URI
   * @param $nex_entities , the entity extracted entities
   * @param $cl_entities , the classification entities
   */
  public static function reconciliateDataTxtNexAndCl($nex_entities, $cl_entities) {
    foreach ($nex_entities as $entity) {
      foreach ($cl_entities as $classification) {
        $uri_ent = $entity->getUriRepresentation();
        $score_detail = $classification->getEntityScoreDetails();
        if (is_array($score_detail)) {
          foreach ($score_detail as $score) {
            if (empty($score->count)) {
              $score->count = 0;
            }
            if ($score->entity == $uri_ent) {
              $score->count++;
            }
          }
        }
        else {
          if (empty($score_detail->count)) {
            $score_detail->count = 0;
          }
          if ($score_detail->entity == $uri_ent) {
            $score_detail->count++;
          }
        }
      }
    }
    return $cl_entities;
  }

  public static function termExistsInVocabulary($tid, $vid) {
    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', 'taxonomy_term');
    $query->propertyCondition('vid', $vid);
    $result = $query->execute();
    if (count($result) == 0) {
      return FALSE;
    }
    else {
      foreach ($result['taxonomy_term'] as $key => $value) {
        if ($tid == $key) {
          return TRUE;
        }
      }
    }
    return FALSE;
  }

  public static function getTranslatedTermName($tid, $language = LANGUAGE_NONE) {
    $wrapper = entity_metadata_wrapper('taxonomy_term', $tid);
    $wrapper_local = $wrapper->language($language);
    if (!empty($wrapper_local->name_field)) {
      $name = $wrapper_local->name_field->value();
    }
    else if (!empty($wrapper_local->name)) {
      $name = $wrapper_local->name->value();
    }
    else {
      $term = taxonomy_term_load($tid);
      $name = $term->name;
    }
    return $name;
  }

  public static function prepareContent($title, $body, $maxlength) {
    $content = '';
    $array = explode(".", wordwrap(strip_tags($body), 400));
    if (!empty($array)) {
      $content .= $title . ' ';
      $total_chars = 0;
      foreach ($array as $elements) {
        $total_chars += strlen($elements);
        if ($total_chars > $maxlength) {
          break;
        }
        $content .= ' ' . $elements;
      }
    }
    return $content;
  }

  /**
   * Retrieve enabled languages
   *
   * Unlike locale_language_list it doesn't require locale module installed.
   *
   * @return: an array with all the enabled language codes
   * 
   */
  public static function getEnabledLangcodes() {
    $list = language_list('enabled');
    return array_keys($list[1]);
  }

  /**
   * Know quickly if we are handling a valid and enabled langcode
   *
   * @return: boolean
   * 
   */
  public static function isValidLangcode($langcode) {
    if (empty($langcode)) {
      return FALSE;
    }
    $result = in_array($langcode, self::getEnabledLangcodes());
    return $result;
  }

  public static function getFieldLanguage($field_name, $language = LANGUAGE_NONE) {
    $field_info = field_info_field($field_name);
    if (field_is_translatable('node', $field_info)) {
      $enabled_languages = field_content_languages();
      if (in_array($language, $enabled_languages)) {
        return $language;
      }
      else {
        return LANGUAGE_NONE;
      }
    }
    else {
      return LANGUAGE_NONE;
    }
  }

}
