<div class="<?php print $classes; ?> clearfix"<?php print $attributes; ?>>
   <?php if (!$page): ?>
    <h2<?php print $title_attributes; ?>>
      <?php if ($teaser): print l($title, 'job/'.$job->jid); else: print $title; endif; ?>
    </h2>
   <?php endif; ?>
  <div class="content"<?php print $content_attributes; ?>>
    <?php
      print render($content);
    ?>
  </div>
</div>