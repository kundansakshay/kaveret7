<?php

/**
 * @file
 * Default theme implementation to display a single Drupal page.
 *
 * Available variables:
 *
 * General utility variables:
 * - $base_path: The base URL path of the Drupal installation. At the very
 *   least, this will always default to /.
 * - $directory: The directory the template is located in, e.g. modules/system
 *   or themes/garland.
 * - $is_front: TRUE if the current page is the front page.
 * - $logged_in: TRUE if the user is registered and signed in.
 * - $is_admin: TRUE if the user has permission to access administration pages.
 *
 * Site identity:
 * - $front_page: The URL of the front page. Use this instead of $base_path,
 *   when linking to the front page. This includes the language domain or
 *   prefix.
 * - $logo: The path to the logo image, as defined in theme configuration.
 * - $site_name: The name of the site, empty when display has been disabled
 *   in theme settings.
 * - $site_slogan: The slogan of the site, empty when display has been disabled
 *   in theme settings.
 *
 * Navigation:
 * - $main_menu (array): An array containing the Main menu links for the
 *   site, if they have been configured.
 * - $secondary_menu (array): An array containing the Secondary menu links for
 *   the site, if they have been configured.
 * - $breadcrumb: The breadcrumb trail for the current page.
 *
 * Page content (in order of occurrence in the default page.tpl.php):
 * - $title_prefix (array): An array containing additional output populated by
 *   modules, intended to be displayed in front of the main title tag that
 *   appears in the template.
 * - $title: The page title, for use in the actual HTML content.
 * - $title_suffix (array): An array containing additional output populated by
 *   modules, intended to be displayed after the main title tag that appears in
 *   the template.
 * - $messages: HTML for status and error messages. Should be displayed
 *   prominently.
 * - $tabs (array): Tabs linking to any sub-pages beneath the current page
 *   (e.g., the view and edit tabs when displaying a node).
 * - $action_links (array): Actions local to the page, such as 'Add menu' on the
 *   menu administration interface.
 * - $feed_icons: A string of all feed icons for the current page.
 * - $node: The node object, if there is an automatically-loaded node
 *   associated with the page, and the node ID is the second argument
 *   in the page's path (e.g. node/12345 and node/12345/revisions, but not
 *   comment/reply/12345).
 *
 * Regions:
 * - $page['help']: Dynamic help text, mostly for admin pages.
 * - $page['highlighted']: Items for the highlighted content region.
 * - $page['content_top']: Items for the header region.
 * - $page['content']: The main content of the current page.
 * - $page['content_bottom']: Items for the header region.
 * - $page['sidebar_first']: Items for the first sidebar.
 * - $page['sidebar_second']: Items for the second sidebar.
 * - $page['header']: Items for the header region.
 * - $page['footer']: Items for the footer region.
 *
 * @see template_preprocess()
 * @see template_preprocess_page()
 * @see template_process()
 */
?>
<?php if ($page['floating']): ?>
    <div id="floating">
 <?php print render($page['floating']); ?>
 </div><!-- /#floating -->
 <?php endif; ?>


  <?php if ($logo || $site_name || $site_slogan || $page['header']): ?>
    <header id="header" role="banner">
      <?php if ($logo): ?>
        <a href="<?php print $front_page; ?>" title="<?php print t('Home'); ?>" rel="home" id="logo">
          <img src="<?php print $logo; ?>" alt="<?php print t('Home'); ?>" />
        </a>
      <?php endif; ?>

      <?php if ($site_name): ?>
          <h1 id="site-name">
            <a href="<?php print $front_page; ?>" title="<?php print t('Home'); ?>" rel="home"><span><?php print $site_name; ?></span></a>
          </h1>
      <?php endif; ?>

      <?php if ($site_slogan): ?>
        <div id="site-slogan"><?php print $site_slogan; ?></div>
      <?php endif; ?>
      <?php if (isset($userpic_social))print $userpic_social; ?>

      <?php print render($page['header']); ?>
    </header><!-- /#header -->
  <?php endif; ?>

<div id="page">
 <div id="main">
    <?php if ($tabs): ?>
      <div class="tabs"><?php print render($tabs); ?></div>
    <?php endif; ?>
    <?php if ($page['highlighted']): ?>
      <div id="highlighted">
        <?php print render($page['highlighted']); ?>
      </div><!-- /#highlighted -->
    <?php endif; ?>
    <?php print render($title_prefix); ?>
    <?php print render($title_suffix); ?>

    <?php print $messages; ?>
    <?php print render($page['help']); ?>
    <?php if ($action_links): ?>
      <ul class="action-links"><?php print render($action_links); ?></ul>
    <?php endif; ?>

  <?php if ($page['sidebar_first']): ?>
    <aside id="sidebar-first" class="sidebar" role="complementary">
      <?php print render($page['sidebar_first']); ?>
    </aside><!-- /#sidebar-first -->
  <?php endif; ?>
  <?php if ($page['sidebar_second']): ?>
    <aside id="sidebar-second" class="sidebar" role="complementary">
      <?php print render($page['sidebar_second']); ?>
    </aside><!-- /#sidebar-second -->
  <?php endif; ?>

    <img height="267" src="<?php print path_to_theme(); ?>/images/top_img.png" width="722" />
    <?php print render($page['content']); ?>

    <?php print $feed_icons; ?>
  </div><!-- /#main -->


</div><!-- /#page -->

<footer id="footer">
  <?php if ($secondary_menu_links): ?>
  <nav id="secondary-menu" role="navigation">
    <?php print $secondary_menu_links ?>
  </nav> <!-- /#secondary-menu -->
  <?php endif; ?>
  <?php print render($footer_icons); ?>
  <?php print render($share_icons); ?>
  <?php //this is the blocks in the footer region
    print render($page['footer']);
  ?>
</footer><!-- /#footer -->

