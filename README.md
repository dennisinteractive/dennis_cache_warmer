# Dennis Cache Warmer
Warm the cache while maintenance mode is still on.


Bypassing maintenance mode requires admin access.

* Either an existing admin account can be used, in which case both the username & password must be passed as options: `drush dennis-cache-warm --username=admin --password=password http://auth.example.com`

* Or let drush create a temporary admin user:`drush dennis-cache-warm --autouser http://auth.example.com` This requires drush to be running on the same server as the site.

To use custom paths, rather than the built in ones ( _/, /news, /reviews, /contact-us_ ) give the **--paths** option the absolute path to a text file that has paths listed, one per line: `drush dennis-cache-warm --autouser --paths=/home/vagrant/repos/dennis_distro_7/sites/autoexpressuk/warm_cache.txt http://auth.example.com`
