<?php
/**
 * @file
 * Template to render a single dataTXT wiki item.
 *
 * @var array $entity
 */
?>
<div class="item">
  <a class="title"><?php print $entity['title'] ?></a>
  <div class="uri"><?php print urldecode($entity['uri']) ?></div>
  <?php if (!empty($entity['abstract']) || !empty($entity['image'])): ?>
    <div class="tip"><div class="inner">
      <?php if (!empty($entity['image'])): ?>
        <img alt="<?php print addslashes($entity['title']) ?>" src="<?php print $entity['image']['thumbnail'] ?>" />
      <?php endif; ?>
      <?php if (!empty($entity['abstract'])): ?>
        <?php print $entity['abstract'] ?>
      <?php endif; ?>
    </div></div>
  <?php endif; ?>
</div>
