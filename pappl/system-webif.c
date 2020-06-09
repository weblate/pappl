//
// System web interface functions for the Printer Application Framework
//
// Copyright © 2019-2020 by Michael R Sweet.
// Copyright © 2010-2019 by Apple Inc.
//
// Licensed under Apache License v2.0.  See the file "LICENSE" for more
// information.
//

//
// Include necessary headers...
//

#include "pappl-private.h"
#include <net/if.h>
#include <ifaddrs.h>
#ifdef HAVE_GNUTLS
#  include <gnutls/gnutls.h>
#  include <gnutls/x509.h>
#endif // HAVE_GNUTLS


//
// Local functions...
//

#ifdef HAVE_GNUTLS
static bool	copy_file(pappl_client_t *client, const char *dst, const char *src);
static bool	install_certificate(pappl_client_t *client, const char *crtfile, const char *keyfile);
static bool	make_certificate(pappl_client_t *client, int num_form, cups_option_t *form);
static bool	make_certsignreq(pappl_client_t *client, int num_form, cups_option_t *form, char *crqpath, size_t crqsize);
#endif // HAVE_GNUTLS

static void	system_footer(pappl_client_t *client);
static void	system_header(pappl_client_t *client, const char *title);


//
// Local globals...
//

#ifdef HAVE_GNUTLS
static const char * const countries[][2] =
{					// List of countries and their ISO 3166 2-letter codes
  { "AF", "Afghanistan" },
  { "AX", "Åland Islands" },
  { "AL", "Albania" },
  { "DZ", "Algeria" },
  { "AS", "American Samoa" },
  { "AD", "Andorra" },
  { "AO", "Angola" },
  { "AI", "Anguilla" },
  { "AQ", "Antarctica" },
  { "AG", "Antigua and Barbuda" },
  { "AR", "Argentina" },
  { "AM", "Armenia" },
  { "AW", "Aruba" },
  { "AU", "Australia" },
  { "AT", "Austria" },
  { "AZ", "Azerbaijan" },
  { "BS", "Bahamas" },
  { "BH", "Bahrain" },
  { "BD", "Bangladesh" },
  { "BB", "Barbados" },
  { "BY", "Belarus" },
  { "BE", "Belgium" },
  { "BZ", "Belize" },
  { "BJ", "Benin" },
  { "BM", "Bermuda" },
  { "BT", "Bhutan" },
  { "BO", "Bolivia (Plurinational State of)" },
  { "BQ", "Bonaire, Sint Eustatius and Saba" },
  { "BA", "Bosnia and Herzegovina" },
  { "BW", "Botswana" },
  { "BV", "Bouvet Island" },
  { "BR", "Brazil" },
  { "IO", "British Indian Ocean Territory" },
  { "BN", "Brunei Darussalam" },
  { "BG", "Bulgaria" },
  { "BF", "Burkina Faso" },
  { "BI", "Burundi" },
  { "CV", "Cabo Verde" },
  { "KH", "Cambodia" },
  { "CM", "Cameroon" },
  { "CA", "Canada" },
  { "KY", "Cayman Islands" },
  { "CF", "Central African Republic" },
  { "TD", "Chad" },
  { "CL", "Chile" },
  { "CN", "China" },
  { "CX", "Christmas Island" },
  { "CC", "Cocos (Keeling) Islands" },
  { "CO", "Colombia" },
  { "KM", "Comoros" },
  { "CD", "Congo, Democratic Republic of the" },
  { "CG", "Congo" },
  { "CK", "Cook Islands" },
  { "CR", "Costa Rica" },
  { "CI", "Côte d'Ivoire" },
  { "HR", "Croatia" },
  { "CU", "Cuba" },
  { "CW", "Curaçao" },
  { "CY", "Cyprus" },
  { "CZ", "Czechia" },
  { "DK", "Denmark" },
  { "DJ", "Djibouti" },
  { "DM", "Dominica" },
  { "DO", "Dominican Republic" },
  { "EC", "Ecuador" },
  { "EG", "Egypt" },
  { "SV", "El Salvador" },
  { "GQ", "Equatorial Guinea" },
  { "ER", "Eritrea" },
  { "EE", "Estonia" },
  { "SZ", "Eswatini" },
  { "ET", "Ethiopia" },
  { "FK", "Falkland Islands (Malvinas)" },
  { "FO", "Faroe Islands" },
  { "FJ", "Fiji" },
  { "FI", "Finland" },
  { "FR", "France" },
  { "GF", "French Guiana" },
  { "PF", "French Polynesia" },
  { "TF", "French Southern Territories" },
  { "GA", "Gabon" },
  { "GM", "Gambia" },
  { "GE", "Georgia" },
  { "DE", "Germany" },
  { "GH", "Ghana" },
  { "GI", "Gibraltar" },
  { "GR", "Greece" },
  { "GL", "Greenland" },
  { "GD", "Grenada" },
  { "GP", "Guadeloupe" },
  { "GU", "Guam" },
  { "GT", "Guatemala" },
  { "GG", "Guernsey" },
  { "GW", "Guinea-Bissau" },
  { "GN", "Guinea" },
  { "GY", "Guyana" },
  { "HT", "Haiti" },
  { "HM", "Heard Island and McDonald Islands" },
  { "VA", "Holy See" },
  { "HN", "Honduras" },
  { "HK", "Hong Kong" },
  { "HU", "Hungary" },
  { "IS", "Iceland" },
  { "IN", "India" },
  { "ID", "Indonesia" },
  { "IR", "Iran (Islamic Republic of)" },
  { "IQ", "Iraq" },
  { "IE", "Ireland" },
  { "IM", "Isle of Man" },
  { "IL", "Israel" },
  { "IT", "Italy" },
  { "JM", "Jamaica" },
  { "JP", "Japan" },
  { "JE", "Jersey" },
  { "JO", "Jordan" },
  { "KZ", "Kazakhstan" },
  { "KE", "Kenya" },
  { "KI", "Kiribati" },
  { "KP", "Korea (Democratic People's Republic of)" },
  { "KR", "Korea, Republic of" },
  { "KW", "Kuwait" },
  { "KG", "Kyrgyzstan" },
  { "LA", "Lao People's Democratic Republic" },
  { "LV", "Latvia" },
  { "LB", "Lebanon" },
  { "LS", "Lesotho" },
  { "LR", "Liberia" },
  { "LY", "Libya" },
  { "LI", "Liechtenstein" },
  { "LT", "Lithuania" },
  { "LU", "Luxembourg" },
  { "MO", "Macao" },
  { "MG", "Madagascar" },
  { "MW", "Malawi" },
  { "MY", "Malaysia" },
  { "MV", "Maldives" },
  { "ML", "Mali" },
  { "MT", "Malta" },
  { "MH", "Marshall Islands" },
  { "MQ", "Martinique" },
  { "MR", "Mauritania" },
  { "MU", "Mauritius" },
  { "YT", "Mayotte" },
  { "MX", "Mexico" },
  { "FM", "Micronesia (Federated States of)" },
  { "MD", "Moldova, Republic of" },
  { "MC", "Monaco" },
  { "MN", "Mongolia" },
  { "ME", "Montenegro" },
  { "MS", "Montserrat" },
  { "MA", "Morocco" },
  { "MZ", "Mozambique" },
  { "MM", "Myanmar" },
  { "NA", "Namibia" },
  { "NR", "Nauru" },
  { "NP", "Nepal" },
  { "NL", "Netherlands" },
  { "NC", "New Caledonia" },
  { "NZ", "New Zealand" },
  { "NI", "Nicaragua" },
  { "NE", "Niger" },
  { "NG", "Nigeria" },
  { "NU", "Niue" },
  { "NF", "Norfolk Island" },
  { "MK", "North Macedonia" },
  { "MP", "Northern Mariana Islands" },
  { "NO", "Norway" },
  { "OM", "Oman" },
  { "PK", "Pakistan" },
  { "PW", "Palau" },
  { "PS", "Palestine, State of" },
  { "PA", "Panama" },
  { "PG", "Papua New Guinea" },
  { "PY", "Paraguay" },
  { "PE", "Peru" },
  { "PH", "Philippines" },
  { "PN", "Pitcairn" },
  { "PL", "Poland" },
  { "PT", "Portugal" },
  { "PR", "Puerto Rico" },
  { "QA", "Qatar" },
  { "RE", "Réunion" },
  { "RO", "Romania" },
  { "RU", "Russian Federation" },
  { "RW", "Rwanda" },
  { "BL", "Saint Barthélemy" },
  { "SH", "Saint Helena, Ascension and Tristan da Cunha" },
  { "KN", "Saint Kitts and Nevis" },
  { "LC", "Saint Lucia" },
  { "MF", "Saint Martin (French part)" },
  { "PM", "Saint Pierre and Miquelon" },
  { "VC", "Saint Vincent and the Grenadines" },
  { "WS", "Samoa" },
  { "SM", "San Marino" },
  { "ST", "Sao Tome and Principe" },
  { "SA", "Saudi Arabia" },
  { "SN", "Senegal" },
  { "RS", "Serbia" },
  { "SC", "Seychelles" },
  { "SL", "Sierra Leone" },
  { "SG", "Singapore" },
  { "SX", "Sint Maarten (Dutch part)" },
  { "SK", "Slovakia" },
  { "SI", "Slovenia" },
  { "SB", "Solomon Islands" },
  { "SO", "Somalia" },
  { "ZA", "South Africa" },
  { "GS", "South Georgia and the South Sandwich Islands" },
  { "SS", "South Sudan" },
  { "ES", "Spain" },
  { "LK", "Sri Lanka" },
  { "SD", "Sudan" },
  { "SR", "Suriname" },
  { "SJ", "Svalbard and Jan Mayen" },
  { "SE", "Sweden" },
  { "CH", "Switzerland" },
  { "SY", "Syrian Arab Republic" },
  { "TW", "Taiwan, Province of China" },
  { "TJ", "Tajikistan" },
  { "TZ", "Tanzania, United Republic of" },
  { "TH", "Thailand" },
  { "TL", "Timor-Leste" },
  { "TG", "Togo" },
  { "TK", "Tokelau" },
  { "TO", "Tonga" },
  { "TT", "Trinidad and Tobago" },
  { "TN", "Tunisia" },
  { "TR", "Turkey" },
  { "TM", "Turkmenistan" },
  { "TC", "Turks and Caicos Islands" },
  { "TV", "Tuvalu" },
  { "UG", "Uganda" },
  { "UA", "Ukraine" },
  { "AE", "United Arab Emirates" },
  { "GB", "United Kingdom of Great Britain and Northern Ireland" },
  { "UK", "United Kingdom" },
  { "UM", "United States Minor Outlying Islands" },
  { "US", "United States of America" },
  { "UY", "Uruguay" },
  { "UZ", "Uzbekistan" },
  { "VU", "Vanuatu" },
  { "VE", "Venezuela (Bolivarian Republic of)" },
  { "VN", "Viet Nam" },
  { "VG", "Virgin Islands (British)" },
  { "VI", "Virgin Islands (U.S.)" },
  { "WF", "Wallis and Futuna" },
  { "EH", "Western Sahara" },
  { "YE", "Yemen" },
  { "ZM", "Zambia" },
  { "ZW", "Zimbabwe" }
};
#endif // HAVE_GNUTLS


//
// '_papplSystemWebConfig()' - Show the system configuration page.
//

void
_papplSystemWebConfig(
    pappl_client_t *client,		// I - Client
    pappl_system_t *system)		// I - System
{
  char		dns_sd_name[64],	// DNS-SD name
		location[128],		// Location
		geo_location[128],	// Geo-location latitude
		organization[128],	// Organization
		org_unit[128];		// Organizational unit
  pappl_contact_t contact;		// Contact info
  const char	*status = NULL;		// Status message, if any


  if (!papplClientHTMLAuthorize(client))
    return;

  if (client->operation == HTTP_STATE_POST)
  {
    int			num_form = 0;	// Number of form variable
    cups_option_t	*form = NULL;	// Form variables

    if ((num_form = papplClientGetForm(client, &form)) == 0)
      status = "Invalid form data.";
    else if (!papplClientValidateForm(client, num_form, form))
      status = "Invalid form submission.";
    else
    {
      _papplSystemWebConfigFinalize(system, num_form, form);

      status = "Changes saved.";
    }

    cupsFreeOptions(num_form, form);
  }

  system_header(client, "Configuration");
  if (status)
    papplClientHTMLPrintf(client, "<div class=\"banner\">%s</div>\n", status);

  _papplClientHTMLInfo(client, true, papplSystemGetDNSSDName(system, dns_sd_name, sizeof(dns_sd_name)), papplSystemGetLocation(system, location, sizeof(location)), papplSystemGetGeoLocation(system, geo_location, sizeof(geo_location)), papplSystemGetOrganization(system, organization, sizeof(organization)), papplSystemGetOrganizationalUnit(system, org_unit, sizeof(org_unit)), papplSystemGetContact(system, &contact));

  papplClientHTMLPuts(client,
                      "        </div>\n"
                      "      </div>\n");

  system_footer(client);
}


//
// '_papplSystemWebConfigFinalize()' - Save the changes to the system configuration.
//

void
_papplSystemWebConfigFinalize(
    pappl_system_t *system,		// I - System
    int            num_form,		// I - Number of form variables
    cups_option_t  *form)		// I - Form variables
{
  const char	*value,			// Form value
		*geo_lat,		// Geo-location latitude
		*geo_lon,		// Geo-location longitude
		*contact_name,		// Contact name
		*contact_email,		// Contact email
		*contact_tel;		// Contact telephone number


  if ((value = cupsGetOption("dns_sd_name", num_form, form)) != NULL)
    papplSystemSetDNSSDName(system, *value ? value : NULL);

  if ((value = cupsGetOption("location", num_form, form)) != NULL)
    papplSystemSetLocation(system, *value ? value : NULL);

  geo_lat = cupsGetOption("geo_location_lat", num_form, form);
  geo_lon = cupsGetOption("geo_location_lon", num_form, form);
  if (geo_lat && geo_lon)
  {
    char	uri[1024];		// "geo:" URI

    if (*geo_lat && *geo_lon)
    {
      snprintf(uri, sizeof(uri), "geo:%g,%g", atof(geo_lat), atof(geo_lon));
      papplSystemSetGeoLocation(system, uri);
    }
    else
      papplSystemSetGeoLocation(system, NULL);
  }

  if ((value = cupsGetOption("organization", num_form, form)) != NULL)
    papplSystemSetOrganization(system, *value ? value : NULL);

  if ((value = cupsGetOption("organizational_unit", num_form, form)) != NULL)
    papplSystemSetOrganizationalUnit(system, *value ? value : NULL);

  contact_name  = cupsGetOption("contact_name", num_form, form);
  contact_email = cupsGetOption("contact_email", num_form, form);
  contact_tel   = cupsGetOption("contact_telephone", num_form, form);
  if (contact_name || contact_email || contact_tel)
  {
    pappl_contact_t	contact;	// Contact info

    memset(&contact, 0, sizeof(contact));

    if (contact_name)
      strlcpy(contact.name, contact_name, sizeof(contact.name));
    if (contact_email)
      strlcpy(contact.email, contact_email, sizeof(contact.email));
    if (contact_tel)
      strlcpy(contact.telephone, contact_tel, sizeof(contact.telephone));

    papplSystemSetContact(system, &contact);
  }
}


//
// '_papplSystemWebHome()' - Show the system home page.
//

void
_papplSystemWebHome(
    pappl_client_t *client,		// I - Client
    pappl_system_t *system)		// I - System
{
  system_header(client, NULL);

  papplClientHTMLPrintf(client,
			"      <div class=\"row\">\n"
			"        <div class=\"col-6\">\n"
			"          <h1 class=\"title\">Configuration <a class=\"btn\" href=\"https://%s:%d/config\">Change</a></h1>\n", client->host_field, client->host_port);

  _papplClientHTMLInfo(client, false, system->dns_sd_name, system->location, system->geo_location, system->organization, system->org_unit, &system->contact);

  _papplSystemWebSettings(client);

  papplClientHTMLPuts(client,
		      "        </div>\n"
                      "        <div class=\"col-6\">\n"
                      "          <h1 class=\"title\">Printers</h1>\n");

  papplSystemIteratePrinters(system, (pappl_printer_cb_t)_papplPrinterIteratorWebCallback, client);

  papplClientHTMLPuts(client,
                      "        </div>\n"
                      "      </div>\n");

  system_footer(client);
}


//
// '_papplSystemWebNetwork()' - Show the system network configuration page.
//

void
_papplSystemWebNetwork(
    pappl_client_t *client,		// I - Client
    pappl_system_t *system)		// I - System
{
  const char	*status = NULL;		// Status message, if any
  struct ifaddrs *addrs,		// List of network addresses
		*addr;			// Current network address


  if (!papplClientHTMLAuthorize(client))
    return;

  if (client->operation == HTTP_STATE_POST)
  {
    int			num_form = 0;	// Number of form variable
    cups_option_t	*form = NULL;	// Form variables

    if ((num_form = papplClientGetForm(client, &form)) == 0)
    {
      status = "Invalid form data.";
    }
    else if (!papplClientValidateForm(client, num_form, form))
    {
      status = "Invalid form submission.";
    }
    else
    {
      const char *value;		// Form variable value

      if ((value = cupsGetOption("hostname", num_form, form)) != NULL)
      {
        // Set hostname and save it...
	papplSystemSetHostname(client->system, value);
        status = "Changes saved.";
      }
    }

    cupsFreeOptions(num_form, form);
  }

  system_header(client, "Networking");

  if (status)
    papplClientHTMLPrintf(client, "<div class=\"banner\">%s</div>\n", status);

  papplClientHTMLStartForm(client, client->uri, false);
  papplClientHTMLPrintf(client,
			"          <table class=\"form\">\n"
			"            <tbody>\n"
			"              <tr><th><label for=\"hostname\">Hostname:</label></th><td><input type=\"text\" name=\"hostname\" value=\"%s\" placeholder=\"name.domain\" pattern=\"^(|[-_a-zA-Z0-9][.-_a-zA-Z0-9]*)$\"> <input type=\"submit\" value=\"Save Changes\"></td></tr>\n", system->hostname);

  if (!getifaddrs(&addrs))
  {
    char	temp[256],		// Address string
		*tempptr;		// Pointer into address

    papplClientHTMLPuts(client, "              <tr><th>IPv4 Addresses:</th><td>");

    for (addr = addrs; addr; addr = addr->ifa_next)
    {
      if (addr->ifa_name == NULL || addr->ifa_addr == NULL || addr->ifa_addr->sa_family != AF_INET || !(addr->ifa_flags & IFF_UP) || (addr->ifa_flags & (IFF_LOOPBACK | IFF_POINTOPOINT)) || !strncmp(addr->ifa_name, "awdl", 4))
        continue;

      httpAddrString((http_addr_t *)addr->ifa_addr, temp, sizeof(temp));
      tempptr = temp;

      if (!strcmp(addr->ifa_name, "wlan0"))
        papplClientHTMLPrintf(client, "Wi-Fi: %s<br>", tempptr);
      else if (!strncmp(addr->ifa_name, "wlan", 4) && isdigit(addr->ifa_name[4]))
        papplClientHTMLPrintf(client, "Wi-Fi %d: %s<br>", atoi(addr->ifa_name + 4) + 1, tempptr);
      else if (!strcmp(addr->ifa_name, "en0") || !strcmp(addr->ifa_name, "eth0"))
        papplClientHTMLPrintf(client, "Ethernet: %s<br>", tempptr);
      else if (!strncmp(addr->ifa_name, "en", 2) && isdigit(addr->ifa_name[2]))
        papplClientHTMLPrintf(client, "Ethernet %d: %s<br>", atoi(addr->ifa_name + 2) + 1, tempptr);
      else if (!strncmp(addr->ifa_name, "eth", 3) && isdigit(addr->ifa_name[3]))
        papplClientHTMLPrintf(client, "Ethernet %d: %s<br>", atoi(addr->ifa_name + 3) + 1, tempptr);
    }

    papplClientHTMLPuts(client,
                        "</td></tr>\n"
                        "              <tr><th>IPv6 Addresses:</th><td>");

    for (addr = addrs; addr; addr = addr->ifa_next)
    {
      if (addr->ifa_name == NULL || addr->ifa_addr == NULL || addr->ifa_addr->sa_family != AF_INET6 || !(addr->ifa_flags & IFF_UP) || (addr->ifa_flags & (IFF_LOOPBACK | IFF_POINTOPOINT)) || !strncmp(addr->ifa_name, "awdl", 4))
        continue;

      httpAddrString((http_addr_t *)addr->ifa_addr, temp, sizeof(temp));

      if ((tempptr = strchr(temp, '+')) != NULL)
        *tempptr = '\0';
      else if ((tempptr = strchr(temp, ']')) != NULL)
        *tempptr = '\0';

      if (!strncmp(temp, "[v1.", 4))
        tempptr = temp + 4;
      else if (*temp == '[')
        tempptr = temp + 1;
      else
        tempptr = temp;

      if (!strcmp(addr->ifa_name, "wlan0"))
        papplClientHTMLPrintf(client, "Wi-Fi: %s<br>", tempptr);
      else if (!strncmp(addr->ifa_name, "wlan", 4) && isdigit(addr->ifa_name[4]))
        papplClientHTMLPrintf(client, "Wi-Fi %d: %s<br>", atoi(addr->ifa_name + 4) + 1, tempptr);
      else if (!strcmp(addr->ifa_name, "en0") || !strcmp(addr->ifa_name, "eth0"))
        papplClientHTMLPrintf(client, "Ethernet: %s<br>", tempptr);
      else if (!strncmp(addr->ifa_name, "en", 2) && isdigit(addr->ifa_name[2]))
        papplClientHTMLPrintf(client, "Ethernet %d: %s<br>", atoi(addr->ifa_name + 2) + 1, tempptr);
      else if (!strncmp(addr->ifa_name, "eth", 3) && isdigit(addr->ifa_name[3]))
        papplClientHTMLPrintf(client, "Ethernet %d: %s<br>", atoi(addr->ifa_name + 3) + 1, tempptr);
    }

    papplClientHTMLPuts(client, "</td></tr>\n");

    freeifaddrs(addrs);
  }

  papplClientHTMLPuts(client,
		      "            </tbody>\n"
		      "          </table>\n"
		      "      </form>\n");

  system_footer(client);
}


//
// '_papplSystemWebSecurity()' - Show the system security (users/password) page.
//

void
_papplSystemWebSecurity(
    pappl_client_t *client,		// I - Client
    pappl_system_t *system)		// I - System
{
  const char	*status = NULL;		// Status message, if any
  struct group	*grp;			// Current group


  if (!papplClientHTMLAuthorize(client))
    return;

  if (client->operation == HTTP_STATE_POST)
  {
    int			num_form = 0;	// Number of form variable
    cups_option_t	*form = NULL;	// Form variables

    if ((num_form = papplClientGetForm(client, &form)) == 0)
    {
      status = "Invalid form data.";
    }
    else if (!papplClientValidateForm(client, num_form, form))
    {
      status = "Invalid form submission.";
    }
    else if (!client->system->auth_service)
    {
      const char	*old_password,	// Old password (if any)
			*new_password,	// New password
			*new_password2;	// New password again
      char		hash[1024];	// Hash of password

      old_password  = cupsGetOption("old_password", num_form, form);
      new_password  = cupsGetOption("new_password", num_form, form);
      new_password2 = cupsGetOption("new_password2", num_form, form);

      if (system->password_hash[0] && (!old_password || strcmp(system->password_hash, papplSystemHashPassword(system, system->password_hash, old_password, hash, sizeof(hash)))))
      {
        status = "Wrong old password.";
      }
      else if (!new_password || !new_password2 || strcmp(new_password, new_password2))
      {
        status = "Passwords do not match.";
      }
      else
      {
        const char	*passptr;	// Pointer into password
        bool		have_lower,	// Do we have a lowercase letter?
			have_upper,	// Do we have an uppercase letter?
			have_digit;	// Do we have a number?

        for (passptr = new_password, have_lower = false, have_upper = false, have_digit = false; *passptr; passptr ++)
        {
          if (isdigit(*passptr & 255))
            have_digit = true;
	  else if (islower(*passptr & 255))
	    have_lower = true;
	  else if (isupper(*passptr & 255))
	    have_upper = true;
	}

        if (!have_digit || !have_lower || !have_upper || strlen(new_password) < 8)
        {
          status = "Password must be at least eight characters long and contain at least one uppercase letter, one lowercase letter, and one digit.";
        }
        else
        {
          papplSystemHashPassword(system, NULL, new_password, hash, sizeof(hash));
          papplSystemSetPassword(system, hash);
          status = "Password changed.";
	}
      }
    }
    else
    {
      const char	 *group;	// Current group
      char		buffer[8192];	// Buffer for strings
      struct group	grpbuf,		// Group buffer
			*grp = NULL;	// Admin group


      if ((group = cupsGetOption("admin_group", num_form, form)) != NULL)
      {
        if (getgrnam_r(group, &grpbuf, buffer, sizeof(buffer), &grp) || !grp)
          status = "Bad administration group.";
	else
	  papplSystemSetAdminGroup(system, group);
      }

      if ((group = cupsGetOption("print_group", num_form, form)) != NULL)
      {
        if (getgrnam_r(group, &grpbuf, buffer, sizeof(buffer), &grp) || !grp)
        {
          status = "Bad print group.";
	}
	else
	{
	  papplSystemSetDefaultPrintGroup(system, group);
	  papplSystemIteratePrinters(system, (pappl_printer_cb_t)papplPrinterSetPrintGroup, (void *)group);
	}
      }

      if (!status)
        status = "Group changes saved.";
    }

    cupsFreeOptions(num_form, form);
  }

  system_header(client, "Security");

  if (status)
    papplClientHTMLPrintf(client, "<div class=\"banner\">%s</div>\n", status);

  papplClientHTMLPuts(client,
                      "        </div>\n"
                      "      </div>\n"
                      "      <div class=\"row\">\n");

  if (system->auth_service)
  {
    // Show Users pane for group controls
    papplClientHTMLStartForm(client, client->uri, false);

    papplClientHTMLPuts(client,
			"        <div class=\"col-12\">\n"
			"          <h2 class=\"title\">Users</h2>\n"
			"          <table class=\"form\">\n"
			"            <tbody>\n"
			"              <tr><th><label for=\"admin_group\">Admin Group:</label></th><td><select name\"admin_group\"><option value=\"\">None</option>");

    setgrent();
    while ((grp = getgrent()) != NULL)
    {
      papplClientHTMLPrintf(client, "<option%s>%s</option>", (system->admin_group && !strcmp(grp->gr_name, system->admin_group)) ? " selected" : "", grp->gr_name);
    }

    papplClientHTMLPuts(client,
			"</select></td></tr>\n"
			"              <tr><th><label for=\"print_group\">Print Group:</label></th><td><select name\"print_group\"><option value=\"\">None</option>");

    setgrent();
    while ((grp = getgrent()) != NULL)
    {
      papplClientHTMLPrintf(client, "<option%s>%s</option>", (system->default_print_group && !strcmp(grp->gr_name, system->default_print_group)) ? " selected" : "", grp->gr_name);
    }

    papplClientHTMLPuts(client,
			"</select></td></tr>\n"
			"              <tr><th></th><td><input type=\"submit\" value=\"Save Changes\"></td></tr>\n"
			"            </tbody>\n"
			"          </table>\n"
			"        </div>\n"
			"        </form>\n");
  }
  else if (system->password_hash[0])
  {
    // Show simple access password update form...
    papplClientHTMLStartForm(client, client->uri, false);

    papplClientHTMLPuts(client,
			"        <div class=\"col-12\">\n"
			"          <h2 class=\"title\">Change Access Password</h2>\n"
			"          <table class=\"form\">\n"
			"            <tbody>\n"
			"              <tr><th><label for=\"old_password\">Current Password:</label></th><td><input type=\"password\" name=\"old_password\"></td></tr>\n"
			"              <tr><th><label for=\"new_password\">New Password:</label></th><td><input type=\"password\" name=\"new_password\" placeholder=\"8+, upper+lower+digit\"></td></tr>\n"
			"              <tr><th><label for=\"new_password2\">New Password (again):</label></th><td><input type=\"password\" name=\"new_password2\" placeholder=\"8+, upper+lower+digit\"></td></tr>\n"
			"              <tr><th></th><td><input type=\"submit\" value=\"Change Access Password\"></td></tr>\n"
			"            </tbody>\n"
			"          </table>\n"
			"        </div>\n"
			"        </form>\n");

  }
  else
  {
    // Show simple access password initial setting form...
    papplClientHTMLStartForm(client, client->uri, false);

    papplClientHTMLPuts(client,
			"        <div class=\"col-12\">\n"
			"          <h2 class=\"title\">Set Access Password</h2>\n"
			"          <table class=\"form\">\n"
			"            <tbody>\n"
			"              <tr><th><label for=\"new_password\">Password:</label></th><td><input type=\"password\" name=\"new_password\" placeholder=\"8+, upper+lower+digit\"></td></tr>\n"
			"              <tr><th><label for=\"new_password2\">Password (again):</label></th><td><input type=\"password\" name=\"new_password2\" placeholder=\"8+, upper+lower+digit\"></td></tr>\n"
			"              <tr><th></th><td><input type=\"submit\" value=\"Set Access Password\"></td></tr>\n"
			"            </tbody>\n"
			"          </table>\n"
			"        </div>\n"
			"        </form>\n");
  }

  // Finish up...
  papplClientHTMLPuts(client,
                      "      </div>\n");

  system_footer(client);
}


//
// '_papplSystemWebSettings()' - Show the system settings panel, as needed.
//

void
_papplSystemWebSettings(
    pappl_client_t *client)		// I - Client
{
  if (client->system->options & (PAPPL_SOPTIONS_NETWORK | PAPPL_SOPTIONS_SECURITY | PAPPL_SOPTIONS_TLS))
  {
    papplClientHTMLPuts(client,
                        "          <h2 class=\"title\">Other Settings</h2>\n"
                        "          <div class=\"btn\">");
    if (client->system->options & PAPPL_SOPTIONS_NETWORK)
      papplClientHTMLPrintf(client, "<a class=\"btn\" href=\"https://%s:%d/network\">Network</a> ", client->host_field, client->host_port);
    if (client->system->options & PAPPL_SOPTIONS_SECURITY)
      papplClientHTMLPrintf(client, "<a class=\"btn\" href=\"https://%s:%d/security\">Security</a> ", client->host_field, client->host_port);
#ifdef HAVE_GNUTLS
    if (client->system->options & PAPPL_SOPTIONS_TLS)
      papplClientHTMLPrintf(client,
                            "<a class=\"btn\" href=\"https://%s:%d/tls-install-crt\">Install TLS Certificate</a> "
                            "<a class=\"btn\" href=\"https://%s:%d/tls-new-crt\">Create New TLS Certificate</a> "
                            "<a class=\"btn\" href=\"https://%s:%d/tls-new-csr\">Create TLS Certificate Request</a> ", client->host_field, client->host_port, client->host_field, client->host_port, client->host_field, client->host_port);
#endif // HAVE_GNUTLS
    papplClientHTMLPuts(client, "</div>\n");
  }

  if ((client->system->options & PAPPL_SOPTIONS_LOG) && client->system->logfile && strcmp(client->system->logfile, "-") && strcmp(client->system->logfile, "syslog"))
    papplClientHTMLPuts(client,
                        "          <h2 class=\"title\">Logging</h2>\n"
                        "          <div class=\"btn\"><a class=\"btn\" href=\"/system.log\">View Log File</a></div>\n");
}


#ifdef HAVE_GNUTLS
//
// '_papplSystemWebTLSInstall()' - Show the system TLS certificate installation page.
//

void
_papplSystemWebTLSInstall(
    pappl_client_t *client,		// I - Client
    pappl_system_t *system)		// I - System
{
  const char	*status = NULL;		// Status message, if any


  if (!papplClientHTMLAuthorize(client))
    return;

  if (client->operation == HTTP_STATE_POST)
  {
    int			num_form = 0;	// Number of form variable
    cups_option_t	*form = NULL;	// Form variables

    if ((num_form = papplClientGetForm(client, &form)) == 0)
    {
      status = "Invalid form data.";
    }
    else if (!papplClientValidateForm(client, num_form, form))
    {
      status = "Invalid form submission.";
    }
    else
    {
      const char	*crtfile,	// Certificate file
			*keyfile;	// Private key file
      char		filename[1024];	// Filename

      crtfile = cupsGetOption("certificate", num_form, form);
      keyfile = cupsGetOption("privatekey", num_form, form);

      if (!keyfile)
      {
        char	hostname[256],		// Hostname
	      	*hostptr;		// Pointer into hostname

        strlcpy(hostname, client->system->hostname, sizeof(hostname));
        if ((hostptr = strchr(hostname, '.')) != NULL)
          *hostptr = '\0';

        snprintf(filename, sizeof(filename), "%s/%s.key", client->system->directory, hostname);
        if (!access(filename, R_OK))
          keyfile = filename;
	else
	  status = "Missing private key.";
      }

      if (!status)
      {
        if (install_certificate(client, crtfile, keyfile))
          status = "Certificate installed.";
        else
          status = "Invalid certificate or private key.";
      }
    }

    cupsFreeOptions(num_form, form);
  }

  system_header(client, "Install TLS Certificate");

  if (status)
    papplClientHTMLPrintf(client, "<div class=\"banner\">%s</div>\n", status);

  papplClientHTMLPuts(client,
                      "        </div>\n"
                      "      </div>\n"
                      "      <div class=\"row\">\n");

  papplClientHTMLStartForm(client, client->uri, true);
  papplClientHTMLPuts(client,
		      "        <div class=\"col-12\">\n"
		      "          <p>This form will install a trusted TLS certificate you have obtained from a Certificate Authority ('CA'). Once installed, it will be used immediately.</p>\n"
		      "          <table class=\"form\">\n"
		      "            <tbody>\n"
		      "              <tr><th><label for=\"certificate\">Certificate:</label></th><td><input type=\"file\" name=\"certificate\" accept=\".crt,.pem,application/pem-certificate-chain,application/x-x509-ca-cert,application/octet-stream\" required> (PEM-encoded)</td></tr>\n"
		      "              <tr><th><label for=\"privatekey\">Private Key:</label></th><td><input type=\"file\" name=\"privatekey\" accept=\".key,.pem,application/octet-stream\"> (PEM-encoded, leave unselected to use the key from the last signing request)</td></tr>\n"
		      "              <tr><th></th><td><input type=\"submit\" value=\"Install Certificate\"></td></tr>\n"
		      "            </tbody>\n"
		      "          </table>\n"
		      "        </div>\n"
		      "        </form>\n"
                      "      </div>\n");

  system_footer(client);
}


//
// '_papplSystemWebTLSNew()' - Show the system TLS certificate/request creation page.
//

void
_papplSystemWebTLSNew(
    pappl_client_t *client,		// I - Client
    pappl_system_t *system)		// I - System
{
  int		i;			// Looping var
  const char	*status = NULL;		// Status message, if any
  char		crqpath[256] = "";	// Certificate request file, if any
  bool		success = false;	// Were we successful?


  if (!papplClientHTMLAuthorize(client))
    return;

  if (client->operation == HTTP_STATE_POST)
  {
    int			num_form = 0;	// Number of form variable
    cups_option_t	*form = NULL;	// Form variables

    if ((num_form = papplClientGetForm(client, &form)) == 0)
    {
      status = "Invalid form data.";
    }
    else if (!papplClientValidateForm(client, num_form, form))
    {
      status = "Invalid form submission.";
    }
    else if (!strcmp(client->uri, "/tls-new-crt"))
    {
      if (make_certificate(client, num_form, form))
      {
        status  = "Certificate created.";
        success = true;
      }
      else
        status = "Unable to create certificate.";
    }
    else
    {
      if (make_certsignreq(client, num_form, form, crqpath, sizeof(crqpath)))
      {
        status  = "Certificate request created.";
        success = true;
      }
      else
        status = "Unable to create certificate request.";
    }

    cupsFreeOptions(num_form, form);
  }

  if (!strcmp(client->uri, "/tls-new-crt"))
    system_header(client, "Create New TLS Certificate");
  else
    system_header(client, "Create TLS Certificate Request");

  if (status)
  {
    papplClientHTMLPrintf(client, "          <div class=\"banner\">%s</div>\n", status);

    if (crqpath[0])
      papplClientHTMLPrintf(client, "          <p><a class=\"btn\" href=\"%s\">Download Certificate Request File</a></p>\n", crqpath);

    if (success)
    {
      papplClientHTMLPuts(client,
                          "        </div>\n"
                          "      </div>\n");
      system_footer(client);
      return;
    }
  }

  papplClientHTMLPuts(client,
                      "        </div>\n"
                      "      </div>\n"
                      "      <div class=\"row\">\n");

  papplClientHTMLStartForm(client, client->uri, false);

  if (!strcmp(client->uri, "/tls-new-crt"))
    papplClientHTMLPuts(client,
			"        <div class=\"col-12\">\n"
			"          <p>This form creates a new 'self-signed' TLS certificate for secure printing. Self-signed certificates are not automatically trusted by web browsers.</p>\n"
			"          <table class=\"form\">\n"
			"            <tbody>\n"
			"              <tr><th><label for=\"duration\">Duration:</label></th><td><input type=\"number\" name=\"duration\" min=\"1\" max=\"10\" step=\"1\" value=\"5\" size=\"2\" maxsize=\"2\">&nbsp;years</td></tr>\n");
  else
    papplClientHTMLPuts(client,
			"        <div class=\"col-12\">\n"
			"          <p>This form creates a certificate signing request ('CSR') that you can send to a Certificate Authority ('CA') to obtain a trusted TLS certificate. The private key is saved separately for use with the certificate you get from the CA.</p>\n"
			"          <table class=\"form\">\n"
			"            <tbody>\n");

  papplClientHTMLPrintf(client,
			"              <tr><th><label for=\"level\">Level:</label></th><td><select name=\"level\"><option value=\"rsa-2048\">Good (2048-bit RSA)</option><option value=\"rsa-4096\">Better (4096-bit RSA)</option><option value=\"ecdsa-p384\">Best (384-bit ECC)</option></select></td></tr>\n"
			"              <tr><th><label for=\"email\">EMail (contact):</label></th><td><input type=\"email\" name=\"email\" value=\"%s\" placeholder=\"name@example.com\"></td></tr>\n"
			"              <tr><th><label for=\"organization\">Organization:</label></th><td><input type=\"text\" name=\"organization\" value=\"%s\" placeholder=\"Organization/business name\"></td></tr>\n"
			"              <tr><th><label for=\"organizational_unit\">Organization Unit:</label></th><td><input type=\"text\" name=\"organizational_unit\" value=\"%s\" placeholder=\"Unit, department, etc.\"></td></tr>\n"
			"              <tr><th><label for=\"city\">City/Locality:</label></th><td><input type=\"text\" name=\"city\" placeholder=\"City/town name\">  <button id=\"address_lookup\" onClick=\"event.preventDefault(); navigator.geolocation.getCurrentPosition(setAddress);\">Use My Position</button></td></tr>\n"
			"              <tr><th><label for=\"state\">State/Province:</label></th><td><input type=\"text\" name=\"state\" placeholder=\"State/province name\"></td></tr>\n"
			"              <tr><th><label for=\"country\">Country or Region:</label></th><td><select name=\"country\"><option value="">Choose</option>", system->contact.email, system->organization ? system->organization : "", system->org_unit ? system->org_unit : "");

  for (i = 0; i < (int)(sizeof(countries) / sizeof(countries[0])); i ++)
    papplClientHTMLPrintf(client, "<option value=\"%s\">%s</option>", countries[i][0], countries[i][1]);

  if (!strcmp(client->uri, "/tls-new-crt"))
    papplClientHTMLPuts(client,
			"</select></td></tr>\n"
			"              <tr><th></th><td><input type=\"submit\" value=\"Create New Certificate\"></td></tr>\n");
  else
    papplClientHTMLPuts(client,
			"</select></td></tr>\n"
			"              <tr><th></th><td><input type=\"submit\" value=\"Create Certificate Signing Request\"></td></tr>\n");

  papplClientHTMLPuts(client,
		      "            </tbody>\n"
		      "          </table>\n"
		      "        </div>\n"
		      "        </form>\n"
		      "        <script>\n"
		      "function setAddress(p) {\n"
		      "  let lat = p.coords.latitude.toFixed(4);\n"
		      "  let lon = p.coords.longitude.toFixed(4);\n"
		      "  let xhr = new XMLHttpRequest();\n"
		      "  xhr.open('GET', 'https://nominatim.openstreetmap.org/reverse?format=jsonv2&lat=' + lat + '&lon=' + lon);\n"
		      "  xhr.responseType = 'json';\n"
		      "  xhr.send();\n"
		      "  xhr.onload = function() {\n"
		      "    if (xhr.status == 200) {\n"
		      "      let response = xhr.response;\n"
		      "      document.forms['form']['city'].value = response['address']['city'];\n"
		      "      document.forms['form']['state'].value = response['address']['state'];\n"
		      "      let country = document.forms['form']['country'];\n"
		      "      let cc = response['address']['country_code'].toUpperCase();\n"
		      "      for (i = 0; i < country.length; i ++) {\n"
		      "	if (country[i].value == cc) {\n"
		      "	  country.selectedIndex = i;\n"
		      "	  break;\n"
		      "	}\n"
		      "      }\n"
		      "    } else {\n"
		      "      let button = document.getElementById('address_lookup');\n"
		      "      button.innerHTML = 'Lookup Failed.';\n"
		      "    }\n"
		      "  }\n"
		      "}\n"
		      "        </script>\n"
                      "      </div>\n");

  system_footer(client);
}


//
// 'copy_file()' - Copy a file.
//

static bool				// O - `true` on success, `false` otherwise
copy_file(pappl_client_t *client,	// I - Client
          const char     *dst,		// I - Destination filename
          const char     *src)		// I - Source filename
{
  cups_file_t	*dstfile,		// Destination file
		*srcfile;		// Source file
  char		buffer[32768];		// Copy buffer
  ssize_t	bytes;			// Bytes to copy


  if ((dstfile = cupsFileOpen(dst, "wb")) == NULL)
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Unable to create file '%s': %s", dst, strerror(errno));
    return (false);
  }

  if ((srcfile = cupsFileOpen(src, "rb")) == NULL)
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Unable to open file '%s': %s", src, strerror(errno));
    cupsFileClose(dstfile);
    unlink(dst);
    return (false);
  }

  while ((bytes = cupsFileRead(srcfile, buffer, sizeof(buffer))) > 0)
  {
    if (cupsFileWrite(dstfile, buffer, (size_t)bytes) < 0)
    {
      papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Unable to write file '%s': %s", dst, strerror(errno));
      cupsFileClose(dstfile);
      unlink(dst);
      cupsFileClose(srcfile);
      return (false);
    }
  }

  cupsFileClose(dstfile);
  cupsFileClose(srcfile);

  return (true);
}


//
// 'install_certificate()' - Install a certificate and private key.
//

static bool				// O - `true` on success, `false` otherwise
install_certificate(
    pappl_client_t *client,		// I - Client
    const char     *crtfile,		// I - PEM-encoded certificate filename
    const char     *keyfile)		// I - PEM-encoded private key filename
{
  pappl_system_t *system = papplClientGetSystem(client);
					// System
  const char	*home;			// Home directory
  char		hostname[256],		// Hostname
		basedir[256],		// CUPS directory
		ssldir[256],		// CUPS "ssl" directory
		dstcrt[1024],		// Destination certificate
		dstkey[1024];		// Destination private key
  gnutls_certificate_credentials_t *credentials;
					// TLS credentials
  int		status;			// Status for loading of credentials


  // Try loading the credentials...
  if ((credentials = (gnutls_certificate_credentials_t *)malloc(sizeof(gnutls_certificate_credentials_t))) == NULL)
    return (false);

  gnutls_certificate_allocate_credentials(credentials);

  status = gnutls_certificate_set_x509_key_file(*credentials, crtfile, keyfile, GNUTLS_X509_FMT_PEM);
  gnutls_certificate_free_credentials(*credentials);
  free(credentials);

  if (status != 0)
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Unable to load TLS credentials: %s", gnutls_strerror(status));
    return (false);
  }

  // If everything checks out, copy the certificate and private key to the
  // CUPS "ssl" directory...
  home = getuid() ? getenv("HOME") : NULL;
  if (home)
    snprintf(basedir, sizeof(basedir), "%s/.cups", home);
  else
    strlcpy(basedir, "/etc/cups", sizeof(basedir));

  if (access(basedir, X_OK))
  {
    // Make "~/.cups" or "/etc/cups" directory...
    if (mkdir(basedir, 0755))
    {
      papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Unable to create directory '%s': %s", basedir, strerror(errno));
      return (false);
    }
  }

  snprintf(ssldir, sizeof(ssldir), "%s/ssl", ssldir);
  if (access(ssldir, X_OK))
  {
    // Make "~/.cups/ssl" or "/etc/cups/ssl" directory...
    if (mkdir(ssldir, 0755))
    {
      papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Unable to create directory '%s': %s", ssldir, strerror(errno));
      return (false);
    }
  }

  snprintf(dstkey, sizeof(dstkey), "%s/%s.key", ssldir, papplSystemGetHostname(system, hostname, sizeof(hostname)));
  snprintf(dstcrt, sizeof(dstcrt), "%s/%s.crt", ssldir, hostname);
  if (!copy_file(client, dstkey, keyfile))
  {
    unlink(dstcrt);
    return (false);
  }

  if (!copy_file(client, dstcrt, crtfile))
  {
    unlink(dstkey);
    return (false);
  }

  // If we get this far we are done!
  return (true);
}


//
// 'make_certificate()' - Make a self-signed certificate and private key.
//

static bool				// O - `true` on success, `false` otherwise
make_certificate(
    pappl_client_t *client,		// I - Client
    int            num_form,		// I - Number of form variables
    cups_option_t  *form)		// I - Form variables
{
  int		i;			// Looping var
  pappl_system_t *system = papplClientGetSystem(client);
					// System
  const char	*home,			// Home directory
		*value,			// Value from form variables
		*level,			// Level/algorithm+bits
		*email,			// Email address
		*organization,		// Organization name
		*org_unit,		// Organizational unit, if any
		*city,			// City/locality
		*state,			// State/province
		*country;		// Country
  int		duration;		// Duration in years
  int		num_alt_names = 1;	// Alternate names
  char		alt_names[4][256];	// Subject alternate names
  char		hostname[256],		// Hostname
		*domain,		// Domain name
		basedir[256],		// CUPS directory
		ssldir[256],		// CUPS "ssl" directory
		crtfile[1024],		// Certificate file
		keyfile[1024];		// Private key file
  gnutls_x509_crt_t crt;		// Self-signed certificate
  gnutls_x509_privkey_t key;		// Private/public key pair
  cups_file_t	*fp;			// Key/cert file
  unsigned char	buffer[8192];		// Buffer for key/cert data
  size_t	bytes;			// Number of bytes of data
  unsigned char	serial[4];		// Serial number buffer
  int		status;			// GNU TLS status


  // Verify that we have all of the required form variables...
  if ((value = cupsGetOption("duration", num_form, form)) == NULL)
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Missing 'duration' form field.");
    return (false);
  }
  else if ((duration = atoi(value)) < 1 || duration > 10)
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Bad 'duration'='%s' form field.", value);
    return (false);
  }

  if ((level = cupsGetOption("level", num_form, form)) == NULL)
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Missing 'level' form field.");
    return (false);
  }

  if ((email = cupsGetOption("email", num_form, form)) == NULL)
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Missing 'email' form field.");
    return (false);
  }

  if ((organization = cupsGetOption("organization", num_form, form)) == NULL)
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Missing 'organization' form field.");
    return (false);
  }

  if ((org_unit = cupsGetOption("organizational_unit", num_form, form)) == NULL)
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Missing 'organizational_unit' form field.");
    return (false);
  }

  if ((city = cupsGetOption("city", num_form, form)) == NULL)
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Missing 'city' form field.");
    return (false);
  }

  if ((state = cupsGetOption("state", num_form, form)) == NULL)
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Missing 'state' form field.");
    return (false);
  }

  if ((country = cupsGetOption("country", num_form, form)) == NULL)
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Missing 'country' form field.");
    return (false);
  }

  // Get all of the names this system is known by...
  papplSystemGetHostname(system, hostname, sizeof(hostname));
  if ((domain = strchr(hostname, '.')) != NULL)
  {
    // If the domain name is not hostname.local or hostname.lan, make that the
    // second Subject Alternate Name...
    if (strcmp(domain, ".local") && strcmp(domain, ".lan"))
      strlcpy(alt_names[num_alt_names ++], hostname, sizeof(alt_names[0]));

    *domain = '\0';
  }

  // then add hostname as the first alternate name...
  strlcpy(alt_names[0], hostname, sizeof(alt_names[0]));

  // and finish up with hostname.lan and hostname.local as the final alternates...
  snprintf(alt_names[num_alt_names ++], sizeof(alt_names[0]), "%s.lan", hostname);
  snprintf(alt_names[num_alt_names ++], sizeof(alt_names[0]), "%s.local", hostname);

  // Store the certificate and private key in the CUPS "ssl" directory...
  home = getuid() ? getenv("HOME") : NULL;
  if (home)
    snprintf(basedir, sizeof(basedir), "%s/.cups", home);
  else
    strlcpy(basedir, "/etc/cups", sizeof(basedir));

  if (access(basedir, X_OK))
  {
    // Make "~/.cups" or "/etc/cups" directory...
    if (mkdir(basedir, 0755))
    {
      papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Unable to create directory '%s': %s", basedir, strerror(errno));
      return (false);
    }
  }

  snprintf(ssldir, sizeof(ssldir), "%s/ssl", basedir);
  if (access(ssldir, X_OK))
  {
    // Make "~/.cups/ssl" or "/etc/cups/ssl" directory...
    if (mkdir(ssldir, 0755))
    {
      papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Unable to create directory '%s': %s", ssldir, strerror(errno));
      return (false);
    }
  }

  snprintf(keyfile, sizeof(keyfile), "%s/%s.key", ssldir, hostname);
  snprintf(crtfile, sizeof(crtfile), "%s/%s.crt", ssldir, hostname);

  papplLogClient(client, PAPPL_LOGLEVEL_DEBUG, "Creating crtfile='%s', keyfile='%s'.", crtfile, keyfile);

  // Create the paired encryption keys...
  gnutls_x509_privkey_init(&key);

  if (!strcmp(level, "rsa-2048"))
    gnutls_x509_privkey_generate(key, GNUTLS_PK_RSA, 2048, 0);
  else if (!strcmp(level, "rsa-4096"))
    gnutls_x509_privkey_generate(key, GNUTLS_PK_RSA, 4096, 0);
  else
    gnutls_x509_privkey_generate(key, GNUTLS_PK_ECDSA, 384, 0);

  // Save the private key...
  bytes = sizeof(buffer);

  if ((status = gnutls_x509_privkey_export(key, GNUTLS_X509_FMT_PEM, buffer, &bytes)) < 0)
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Unable to export private key: %s", gnutls_strerror(status));
    gnutls_x509_privkey_deinit(key);
    return (false);
  }
  else if ((fp = cupsFileOpen(keyfile, "w")) != NULL)
  {
    cupsFileWrite(fp, (char *)buffer, bytes);
    cupsFileClose(fp);
  }
  else
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Unable to create private key file '%s': %s", keyfile, strerror(errno));
    gnutls_x509_privkey_deinit(key);
    return (false);
  }

  // Create the self-signed certificate...
  i         = (int)(time(NULL) / 60);
  serial[0] = i >> 24;
  serial[1] = i >> 16;
  serial[2] = i >> 8;
  serial[3] = i;

  gnutls_x509_crt_init(&crt);
  gnutls_x509_crt_set_dn_by_oid(crt, GNUTLS_OID_X520_COUNTRY_NAME, 0, country, (unsigned)strlen(country));
  gnutls_x509_crt_set_dn_by_oid(crt, GNUTLS_OID_X520_COMMON_NAME, 0, hostname, (unsigned)strlen(hostname));
  gnutls_x509_crt_set_dn_by_oid(crt, GNUTLS_OID_X520_ORGANIZATION_NAME, 0, organization, (unsigned)strlen(organization));
  gnutls_x509_crt_set_dn_by_oid(crt, GNUTLS_OID_X520_ORGANIZATIONAL_UNIT_NAME, 0, org_unit, (unsigned)strlen(org_unit));
  gnutls_x509_crt_set_dn_by_oid(crt, GNUTLS_OID_X520_STATE_OR_PROVINCE_NAME, 0, state, (unsigned)strlen(state));
  gnutls_x509_crt_set_dn_by_oid(crt, GNUTLS_OID_X520_LOCALITY_NAME, 0, city, (unsigned)strlen(city));
  gnutls_x509_crt_set_dn_by_oid(crt, GNUTLS_OID_PKCS9_EMAIL, 0, email, (unsigned)strlen(email));
  gnutls_x509_crt_set_key(crt, key);
  gnutls_x509_crt_set_serial(crt, serial, sizeof(serial));
  gnutls_x509_crt_set_activation_time(crt, time(NULL));
  gnutls_x509_crt_set_expiration_time(crt, time(NULL) + duration * 365 * 86400);
  gnutls_x509_crt_set_ca_status(crt, 0);
  gnutls_x509_crt_set_subject_alt_name(crt, GNUTLS_SAN_DNSNAME, alt_names[0], (unsigned)strlen(alt_names[0]), GNUTLS_FSAN_SET);
  for (i = 1; i < num_alt_names; i ++)
    gnutls_x509_crt_set_subject_alt_name(crt, GNUTLS_SAN_DNSNAME, alt_names[i], (unsigned)strlen(alt_names[i]), GNUTLS_FSAN_APPEND);
  gnutls_x509_crt_set_key_purpose_oid(crt, GNUTLS_KP_TLS_WWW_SERVER, 0);
  gnutls_x509_crt_set_key_usage(crt, GNUTLS_KEY_DIGITAL_SIGNATURE | GNUTLS_KEY_KEY_ENCIPHERMENT);
  gnutls_x509_crt_set_version(crt, 3);

  bytes = sizeof(buffer);
  if (gnutls_x509_crt_get_key_id(crt, 0, buffer, &bytes) >= 0)
    gnutls_x509_crt_set_subject_key_id(crt, buffer, bytes);

  gnutls_x509_crt_sign(crt, crt, key);

  // Save the certificate and public key...
  bytes = sizeof(buffer);
  if ((status = gnutls_x509_crt_export(crt, GNUTLS_X509_FMT_PEM, buffer, &bytes)) < 0)
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Unable to export public key and X.509 certificate: %s", gnutls_strerror(status));
    gnutls_x509_crt_deinit(crt);
    gnutls_x509_privkey_deinit(key);
    return (false);
  }
  else if ((fp = cupsFileOpen(crtfile, "w")) != NULL)
  {
    cupsFileWrite(fp, (char *)buffer, bytes);
    cupsFileClose(fp);
  }
  else
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Unable to create public key and X.509 certificate file '%s': %s", crtfile, strerror(errno));
    gnutls_x509_crt_deinit(crt);
    gnutls_x509_privkey_deinit(key);
    return (false);
  }

  // Now create symlinks for each of the alternate names...
  for (i = 1; i < num_alt_names; i ++)
  {
    char altfile[1024];			// Alternate cert/key filename

    snprintf(altfile, sizeof(altfile), "%s/%s.key", ssldir, alt_names[i]);
    unlink(altfile);
    symlink(keyfile, altfile);

    snprintf(altfile, sizeof(altfile), "%s/%s.crt", ssldir, alt_names[i]);
    unlink(altfile);
    symlink(crtfile, altfile);
  }

  // If we get this far we are done!
  gnutls_x509_crt_deinit(crt);
  gnutls_x509_privkey_deinit(key);

  return (true);
}


//
// 'make_certsignreq()' - Make a certificate signing request and private key.
//

static bool				// O - `true` on success, `false` otherwise
make_certsignreq(
    pappl_client_t *client,		// I - Client
    int            num_form,		// I - Number of form variables
    cups_option_t  *form,		// I - Form variables
    char           *crqpath,		// I - Certificate request filename buffer
    size_t         crqsize)		// I - Size of certificate request buffer
{
  pappl_system_t *system = papplClientGetSystem(client);
					// System
  const char	*level,			// Level/algorithm+bits
		*email,			// Email address
		*organization,		// Organization name
		*org_unit,		// Organizational unit, if any
		*city,			// City/locality
		*state,			// State/province
		*country;		// Country
  char		hostname[256],		// Hostname
		crqfile[1024],		// Certificate request file
		keyfile[1024];		// Private key file
  gnutls_x509_crq_t crq;		// Certificate request
  gnutls_x509_privkey_t key;		// Private/public key pair
  cups_file_t	*fp;			// Key/cert file
  unsigned char	buffer[8192];		// Buffer for key/cert data
  size_t	bytes;			// Number of bytes of data
  int		status;			// GNU TLS status


  *crqpath = '\0';

  // Verify that we have all of the required form variables...
  if ((level = cupsGetOption("level", num_form, form)) == NULL)
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Missing 'level' form field.");
    return (false);
  }

  if ((email = cupsGetOption("email", num_form, form)) == NULL)
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Missing 'email' form field.");
    return (false);
  }

  if ((organization = cupsGetOption("organization", num_form, form)) == NULL)
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Missing 'organization' form field.");
    return (false);
  }

  if ((org_unit = cupsGetOption("organizational_unit", num_form, form)) == NULL)
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Missing 'organizational_unit' form field.");
    return (false);
  }

  if ((city = cupsGetOption("city", num_form, form)) == NULL)
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Missing 'city' form field.");
    return (false);
  }

  if ((state = cupsGetOption("state", num_form, form)) == NULL)
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Missing 'state' form field.");
    return (false);
  }

  if ((country = cupsGetOption("country", num_form, form)) == NULL)
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Missing 'country' form field.");
    return (false);
  }

  // Store the certificate request and private key in the spool directory...
  snprintf(keyfile, sizeof(keyfile), "%s/%s.key", system->directory, papplSystemGetHostname(system, hostname, sizeof(hostname)));
  snprintf(crqfile, sizeof(crqfile), "%s/%s.csr", system->directory, hostname);
  snprintf(crqpath, crqsize, "/%s.csr", hostname);

  // Create the paired encryption keys...
  gnutls_x509_privkey_init(&key);

  if (!strcmp(level, "rsa-2048"))
    gnutls_x509_privkey_generate(key, GNUTLS_PK_RSA, 2048, 0);
  else if (!strcmp(level, "rsa-4096"))
    gnutls_x509_privkey_generate(key, GNUTLS_PK_RSA, 4096, 0);
  else
    gnutls_x509_privkey_generate(key, GNUTLS_PK_ECDSA, 384, 0);

  // Save the private key...
  bytes = sizeof(buffer);

  if ((status = gnutls_x509_privkey_export(key, GNUTLS_X509_FMT_PEM, buffer, &bytes)) < 0)
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Unable to export private key: %s", gnutls_strerror(status));
    gnutls_x509_privkey_deinit(key);
    return (false);
  }
  else if ((fp = cupsFileOpen(keyfile, "w")) != NULL)
  {
    cupsFileWrite(fp, (char *)buffer, bytes);
    cupsFileClose(fp);
  }
  else
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Unable to create private key file '%s': %s", keyfile, strerror(errno));
    gnutls_x509_privkey_deinit(key);
    return (false);
  }

  // Create the certificate request...
  gnutls_x509_crq_init(&crq);
  gnutls_x509_crq_set_dn_by_oid(crq, GNUTLS_OID_X520_COUNTRY_NAME, 0, country, (unsigned)strlen(country));
  gnutls_x509_crq_set_dn_by_oid(crq, GNUTLS_OID_X520_COMMON_NAME, 0, hostname, (unsigned)strlen(hostname));
  gnutls_x509_crq_set_dn_by_oid(crq, GNUTLS_OID_X520_ORGANIZATION_NAME, 0, organization, (unsigned)strlen(organization));
  gnutls_x509_crq_set_dn_by_oid(crq, GNUTLS_OID_X520_ORGANIZATIONAL_UNIT_NAME, 0, org_unit, (unsigned)strlen(org_unit));
  gnutls_x509_crq_set_dn_by_oid(crq, GNUTLS_OID_X520_STATE_OR_PROVINCE_NAME, 0, state, (unsigned)strlen(state));
  gnutls_x509_crq_set_dn_by_oid(crq, GNUTLS_OID_X520_LOCALITY_NAME, 0, city, (unsigned)strlen(city));
  gnutls_x509_crq_set_dn_by_oid(crq, GNUTLS_OID_PKCS9_EMAIL, 0, email, (unsigned)strlen(email));
  gnutls_x509_crq_set_key(crq, key);
  gnutls_x509_crq_set_subject_alt_name(crq, GNUTLS_SAN_DNSNAME, hostname, (unsigned)strlen(hostname), GNUTLS_FSAN_SET);
  gnutls_x509_crq_set_key_purpose_oid(crq, GNUTLS_KP_TLS_WWW_SERVER, 0);
  gnutls_x509_crq_set_key_usage(crq, GNUTLS_KEY_DIGITAL_SIGNATURE | GNUTLS_KEY_KEY_ENCIPHERMENT);
  gnutls_x509_crq_set_version(crq, 3);

  gnutls_x509_crq_sign2(crq, key, GNUTLS_DIG_SHA256, 0);

  // Save the certificate request and public key...
  bytes = sizeof(buffer);
  if ((status = gnutls_x509_crq_export(crq, GNUTLS_X509_FMT_PEM, buffer, &bytes)) < 0)
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Unable to export public key and X.509 certificate request: %s", gnutls_strerror(status));
    gnutls_x509_crq_deinit(crq);
    gnutls_x509_privkey_deinit(key);
    return (false);
  }
  else if ((fp = cupsFileOpen(crqfile, "w")) != NULL)
  {
    cupsFileWrite(fp, (char *)buffer, bytes);
    cupsFileClose(fp);
  }
  else
  {
    papplLogClient(client, PAPPL_LOGLEVEL_ERROR, "Unable to create public key and X.509 certificate request file '%s': %s", crqfile, strerror(errno));
    gnutls_x509_crq_deinit(crq);
    gnutls_x509_privkey_deinit(key);
    return (false);
  }

  // If we get this far we are done!
  papplSystemAddResourceFile(system, crqpath, "application/pkcs10", crqfile);

  gnutls_x509_crq_deinit(crq);
  gnutls_x509_privkey_deinit(key);

  return (true);
}
#endif // HAVE_GNUTLS


//
// 'system_footer()' - Show the system footer.
//

static void
system_footer(pappl_client_t *client)	// I - Client
{
  papplClientHTMLPuts(client, "    </div>\n");

  papplClientHTMLFooter(client);
}


//
// 'system_header()' - Show the system header.
//

static void
system_header(pappl_client_t *client,	// I - Client
              const char     *title)	// I - Title
{
  if (!papplClientRespondHTTP(client, HTTP_STATUS_OK, NULL, "text/html", 0, 0))
    return;

  papplClientHTMLHeader(client, title, 0);

  if (client->system->versions[0].sversion[0])
    papplClientHTMLPrintf(client,
			  "    <div class=\"header2\">\n"
			  "      <div class=\"row\">\n"
			  "        <div class=\"col-12 nav\">\n"
			  "          Version %s\n"
			  "        </div>\n"
			  "      </div>\n"
			  "    </div>\n", client->system->versions[0].sversion);

  papplClientHTMLPuts(client, "    <div class=\"content\">\n");

  if (title)
    papplClientHTMLPrintf(client,
			  "      <div class=\"row\">\n"
			  "        <div class=\"col-12\">\n"
			  "          <h1 class=\"title\">%s</h1>\n", title);
}


#if 0
//
// 'device_cb()' - Device callback.
//

static int				// O - 1 to continue
device_cb(const char      *device_uri,	// I - Device URI
          pappl_client_t *client)	// I - Client
{
  char	scheme[32],			// URI scheme
	userpass[32],			// Username/password (unused)
	make[64],			// Make from URI
	model[256],			// Model from URI
	*serial;			// Pointer to serial number
  int	port;				// Port number (unused)


  if (httpSeparateURI(HTTP_URI_CODING_ALL, device_uri, scheme, sizeof(scheme), userpass, sizeof(userpass), make, sizeof(make), &port, model, sizeof(model)) >= HTTP_URI_STATUS_OK)
  {
    if ((serial = strstr(model, "?serial=")) != NULL)
    {
      *serial = '\0';
      serial += 8;
    }

    if (serial)
      papplClientHTMLPrintf(client, "<option value=\"%s\">%s %s (%s)</option>", device_uri, make, model + 1, serial);
    else
      papplClientHTMLPrintf(client, "<option value=\"%s\">%s %s</option>", device_uri, make, model + 1);
  }

  return (1);
}


//
// 'show_delete()' - Show the delete printer page.
//

static int				// O - 1 on success, 0 on failure
show_delete(pappl_client_t *client,	// I - Client connection
            int             printer_id)	// I - Printer ID
{
  pappl_printer_t *printer;		// Printer
  http_status_t	status;			// Authorization status
  int		num_form = 0;		// Number of form variables
  cups_option_t	*form = NULL;		// Form variables
  const char	*session = NULL,	// Session key
		*error = NULL;		// Error message, if any
  char		title[1024];		// Title for page


  if ((printer = lprintFindPrinter(client->system, NULL, printer_id)) == NULL)
  {
    // Printer not found...
    return (papplClientRespondHTTP(client, HTTP_STATUS_NOT_FOUND, NULL, NULL, 0));
  }

  if ((status = lprintIsAuthorized(client)) != HTTP_STATUS_CONTINUE)
  {
    // Need authentication...
    return (papplClientRespondHTTP(client, status, NULL, NULL, 0));
  }

  if (client->operation == HTTP_STATE_POST)
  {
    // Get form data...
    int	valid = 1;

    num_form       = get_form_data(client, &form);
    session        = cupsGetOption("session-key", num_form, form);

    if (!session || strcmp(session, client->system->session_key))
    {
      valid = 0;
      error = "Bad or missing session key.";
    }

    if (valid)
    {
      // Delete printer...
      if (!printer->processing_job)
      {
	lprintDeletePrinter(printer);
	printer = NULL;
      }
      else
	printer->is_deleted = 1;

      if (!client->system->save_time)
	client->system->save_time = time(NULL) + 1;

      papplClientRespondHTTP(client, HTTP_STATUS_OK, NULL, "text/html", 0);

      papplClientHTMLHeader(client, printer ? "Deleting Printer" : "Printer Deleted", 0);
      papplClientHTMLPrintf(client, "<p><button onclick=\"window.location.href='/';\">&lArr; Return to Printers</button></p>\n");
      papplClientHTMLFooter(client);
      return (1);
    }
  }

  papplClientRespondHTTP(client, HTTP_STATUS_OK, NULL, "text/html", 0);

  snprintf(title, sizeof(title), "Delete Printer '%s'", printer->printer_name);
  papplClientHTMLHeader(client, title, 0);
  papplClientHTMLPrintf(client, "<p><button onclick=\"window.location.href='/';\">&lArr; Return to Printers</button></p>\n");

  if (error)
    papplClientHTMLPrintf(client, "<blockquote><em>Error:</em> %s</blockquote>\n", error);

  papplClientHTMLPrintf(client, "<form method=\"POST\" action=\"/delete/%d\">"
                      "<input name=\"session-key\" type=\"hidden\" value=\"%s\">"
		      "<table class=\"form\">\n"
		      "<tr><th>Confirm:</th><td><input type=\"submit\" value=\"Delete Printer '%s'\"></td></tr>\n"
		      "</table></form>\n", printer_id, client->system->session_key, printer->printer_name);
  papplClientHTMLFooter(client);

  cupsFreeOptions(num_form, form);

  return (1);
}


//
// 'show_modify()' - Show the modify printer page.
//

static int				// O - 1 on success, 0 on failure
show_modify(pappl_client_t *client,	// I - Client connection
	    int             printer_id)	// I - Printer ID
{
  pappl_printer_t *printer;		// Printer
  http_status_t	status;			// Authorization status
  int		num_form = 0;		// Number of form variables
  cups_option_t	*form = NULL;		// Form variables
  const char	*session = NULL,	// Session key
		*location = NULL,	// Human-readable location
		*latitude = NULL,	// Latitude
		*longitude = NULL,	// Longitude
		*organization = NULL,	// Organization
		*org_unit = NULL,	// Organizational unit
		*error = NULL;		// Error message, if any
  char		title[1024];		// Title for page
  float		latval = 0.0f,		// Latitude in degrees
		lonval = 0.0f;		// Longitude in degrees


  if ((printer = lprintFindPrinter(client->system, NULL, printer_id)) == NULL)
  {
    // Printer not found...
    return (papplClientRespondHTTP(client, HTTP_STATUS_NOT_FOUND, NULL, NULL, 0));
  }

  if ((status = lprintIsAuthorized(client)) != HTTP_STATUS_CONTINUE)
  {
    // Need authentication...
    return (papplClientRespondHTTP(client, status, NULL, NULL, 0));
  }

  if (client->operation == HTTP_STATE_POST)
  {
    // Get form data...
    int		valid = 1;		// Is form data valid?

    num_form     = get_form_data(client, &form);
    session      = cupsGetOption("session-key", num_form, form);
    location     = cupsGetOption("printer-location", num_form, form);
    latitude     = cupsGetOption("latitude", num_form, form);
    longitude    = cupsGetOption("longitude", num_form, form);
    organization = cupsGetOption("printer-organization", num_form, form);
    org_unit     = cupsGetOption("printer-organizational-unit", num_form, form);

    if (!session || strcmp(session, client->system->session_key))
    {
      valid = 0;
      error = "Bad or missing session key.";
    }

    if (valid && latitude)
    {
      latval = atof(latitude);

      if (*latitude && (!strchr("0123456789.-+", *latitude) || latval < -90.0f || latval > 90.0f))
      {
        valid = 0;
        error = "Bad latitude value.";
      }
    }

    if (valid && longitude)
    {
      lonval = atof(longitude);

      if (*longitude && (!strchr("0123456789.-+", *longitude) || lonval < -180.0f || lonval > 180.0f))
      {
        valid = 0;
        error = "Bad longitude value.";
      }
    }

    if (valid && latitude && longitude && !*latitude != !*longitude)
    {
      valid = 0;
      error = "Both latitude and longitude must be specified.";
    }

    if (valid)
    {
      pthread_rwlock_wrlock(&printer->rwlock);

      if (location)
      {
        free(printer->location);
        printer->location = strdup(location);
      }

      if (latitude && *latitude && longitude && *longitude)
      {
        char geo[1024];			// geo: URI

        snprintf(geo, sizeof(geo), "geo:%g,%g", atof(latitude), atof(longitude));
        free(printer->geo_location);
        printer->geo_location = strdup(geo);
      }
      else if (latitude && longitude)
      {
        free(printer->geo_location);
        printer->geo_location = NULL;
      }

      if (organization)
      {
        free(printer->organization);
        printer->organization = strdup(organization);
      }

      if (org_unit)
      {
        free(printer->org_unit);
        printer->org_unit = strdup(org_unit);
      }

      media_parse("media-ready0", printer->driver->media_ready + 0, num_form, form);
      if (printer->driver->num_source > 1)
        media_parse("media-ready1", printer->driver->media_ready + 1, num_form, form);

      printer->config_time = time(NULL);

      pthread_rwlock_unlock(&printer->rwlock);

      if (!client->system->save_time)
	client->system->save_time = time(NULL) + 1;

      papplClientRespondHTTP(client, HTTP_STATUS_OK, NULL, "text/html", 0);

      papplClientHTMLHeader(client, "Printer Modified", 0);
      papplClientHTMLPrintf(client, "<p><button onclick=\"window.location.href='/';\">&lArr; Return to Printers</button></p>\n");
      papplClientHTMLFooter(client);
      return (1);
    }
  }

  if (!location)
    location = printer->location;

  if (latitude && longitude)
  {
    latval = atof(latitude);
    lonval = atof(longitude);
  }
  else if (printer->geo_location)
    sscanf(printer->geo_location, "geo:%f,%f", &latval, &lonval);

  if (!organization)
    organization = printer->organization;

  if (!org_unit)
    org_unit = printer->org_unit;

  papplClientRespondHTTP(client, HTTP_STATUS_OK, NULL, "text/html", 0);

  snprintf(title, sizeof(title), "Modify Printer '%s'", printer->printer_name);
  papplClientHTMLHeader(client, title, 0);
  papplClientHTMLPrintf(client, "<p><button onclick=\"window.location.href='/';\">&lArr; Return to Printers</button></p>\n");

  if (error)
    papplClientHTMLPrintf(client, "<blockquote><em>Error:</em> %s</blockquote>\n", error);

  papplClientHTMLPrintf(client, "<form method=\"POST\" action=\"/modify/%d\">"
                      "<input name=\"session-key\" type=\"hidden\" value=\"%s\">"
		      "<table class=\"form\">\n", printer_id, client->system->session_key);
  papplClientHTMLPrintf(client, "<tr><th>Location:</th><td><input name=\"printer-location\" value=\"%s\" size=\"32\" placeholder=\"Human-readable location\"></td></tr>\n", location ? location : "");
  papplClientHTMLPrintf(client, "<tr><th>Latitude:</th><td><input name=\"latitude\" type=\"number\" value=\"%g\" min=\"-90\" max=\"90\" step=\"0.000001\" size=\"10\" placeholder=\"Latitude Degrees\"></td></tr>\n", latval);
  papplClientHTMLPrintf(client, "<tr><th>Longitude:</th><td><input name=\"longitude\" type=\"number\" value=\"%g\" min=\"-180\" max=\"180\" step=\"0.000001\" size=\"11\" placeholder=\"Longitude Degrees\"></td></tr>\n", lonval);
  papplClientHTMLPrintf(client, "<tr><th>Organization:</th><td><input name=\"printer-organization\" value=\"%s\" size=\"32\" placeholder=\"Organization name\"></td></tr>\n", organization ? organization : "");
  papplClientHTMLPrintf(client, "<tr><th>Organizational Unit:</th><td><input name=\"printer-organizational-unit\" value=\"%s\" size=\"32\" placeholder=\"Unit/division/group\"></td></tr>\n", org_unit ? org_unit : "");
  media_chooser(client, printer, "Main Roll:", "media-ready0", printer->driver->media_ready + 0);
  if (printer->driver->num_source > 1)
    media_chooser(client, printer, "Second Roll:", "media-ready1", printer->driver->media_ready + 1);
  papplClientHTMLPrintf(client, "<tr><th></th><td><input type=\"submit\" value=\"Modify Printer\"></td></tr>\n"
                      "</table></form>\n");
  papplClientHTMLFooter(client);

  cupsFreeOptions(num_form, form);

  return (1);
}
#endif // 0
