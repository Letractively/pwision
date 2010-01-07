<?php
/*
part of PWision toolkit: http://pwision.googlecode.com/
Copyright (C) 2009 Becheru Petru-Ioan

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
include_once("PMainComponent.inc");
include_once("PElement.inc");
include_once("PMySQLConsole.inc");
include_once("PStructure.inc");
include_once("Users.inc");
include_once("PForm.inc");
include_once("IMessage.inc");
include_once("PStaticSection.inc");

/** 	The PMostBasicWebPage class is designed to be a model the most basic webpage: a PDrawable with a database connection.
	*	\latexonly \label{PWision:PMostBasicWebPage} \index{PMostBasicWebPage} \index{database} \endlatexonly
	*	\brief a PDrawable with database connection.
	*	\version 1.1.0
	*	\par Example:
	*		See PBasicWebPage class.
	*/
class PMostBasicWebPage extends PDrawable
	{
	///database console
	public $DB;
	/**	Starts the database connection, gets the console into $this->DB and selects the database. See PMySQLConsole.
		*	\brief start the database connection.
		*	\par Example code:
\code
protected function StartDBConnection()
	{
	$this->DB=new PMySQLConsole($UserName,$PassWord,$Host);
	$this->DB->SelectDB($DatabaseName);
	}
\endcode
		*/
	protected function StartDBConnection()
		{
		$this->DB=include('StartDBConnection.inc');
		}
	/**	Constructs the PDrawable and starts the database connection.
		*	\brief constructor
		*/
	function PMostBasicWebPage()
		{
		PMainComponent::$Main=&$this;
		PDrawable::PDrawable();
		$this->StartDBConnection();
		}
	}

/** 	The Sitemap class is designed to model the sitemap webpage.
	*	\latexonly \label{PWision:PSitemap} \index{PSitemap} \index{sitemap} \endlatexonly
	*	\brief the sitemap webpage.
	*	\version 1.0.0
	*	\par Example:
\code
include_once("pwision.inc");
class PMySitemap extends PSitemap
	{
	protected function StartDBConnection()
		{
		$this->DB=new PMySQLConsole($UserName,$PassWord,$Host);
		$this->DB->SelectDB($DatabaseName);
		}
	function SiteURL()
		{
		return 'http://www.example.com/';
		}
	}

print( new PMySitemap );
\endcode
	*	\par Output:
\verbatim
http://www.example.com/
http://www.example.com/index.php?section=s1
http://www.example.com/index.php?section=s2
http://www.example.com/index.php?section=s3
\endverbatim
	*/
abstract class PSitemap extends PMostBasicWebPage
	{
	/**	Constructs a textual list of hyperlinks to the sections of the website.
		*	\brief the sitemap txt
		*	\version 1.1.0
		*	\return Accesible sections of the website
		*/
	function Draw()
		{
		/// the links element
		$ul=$this->ListStructure() ;
		/// the list of accessible sections, including the hidden ones
		$SectsArray=Main()->DB->fetch_all_query(
			"SELECT * FROM Sections WHERE ".
					"(".
					"select Count(*) from GroupSections where SectionName=Sections.Name ".
						" and GroupID in ".
							"(select GroupID from GroupUsers where UserID='0')".
					")>0"
			);
		$li=$ul->newKid();
		$li->Insert(
			new PText(
				$this->SiteURL().
				"\n"
				)
			);
		if(count($SectsArray)>0)
			foreach($SectsArray as $section)
				{
				$li=$ul->newKid();
				$li->Insert(
					new PText(
						$this->SiteURL().
						Sections::Link2SQLSection($section)->getAtr('href').
						"\n"
						)
					);
				}
		return $ul->Draw();
		}
	/**	Constructs a list structure.
		*	\brief a list structure.
		*	\return an  PStructure
		*/
	protected function ListStructure()
		{
		return new PStructure( new PContent, new PContent );
		}
	/**
		*	\brief the site's name.
		*	\return an PText containing the site's root url
		*	\par Example:
\code
function SiteURL()
	{
	return 'http://www.example.com/';
	}
\endcode
		*/
	abstract function SiteURL();
	}


/** 	The PBasicWebPage class is designed to model a webpage. Main atributes of a webpage are: title, motto, message, section and the language.
	*	\latexonly \label{PWision:PBasicWebPage} \index{PBasicWebPage} \index{webpage} \index{section} \endlatexonly
	*	\brief a basic web page.
	*	\version 1.7.0
	*	\note  Themed webpages will extend this class.
	*	\par Example:
	*		see PCSSZenWebPage class.
	*/
abstract class PBasicWebPage extends PMostBasicWebPage implements IMessage
	{
	/**	\brief textual representation of the object
		*	\return a string
		*/
	function Draw()
		{
		/// the html element
		$html=Object(new PElement("html"))
				->setAtr("xmlns","http://www.w3.org/1999/xhtml")
			->Insert($this->Head())
			->Insert($this->Body())
			;
		return
			"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" ".
			"\"http://www.w3.org/TR/xhtml11/DTD/xhtml1.dtd\">\n".
			$html->Draw();
		}
	/**	Constructs a web page as a html element and inserts the head and the body.
		*	\brief constructor
		*	\version 1.0.1
		*/
	function PBasicWebPage()
		{
		PMostBasicWebPage::PMostBasicWebPage();
		$this->ProcessSessionAndArguments();
		}
	/// Current User Information
	public $UserInfo;
	/// Last Emitted Message
	private $Message="";
		/**	Adds a new message.
			*	\brief Adds a new message.
			*	\param $text the message text
			*	\return reference to $this
			*/
		function AddMessage($text)
			{
			$this->Message.=($text."<br/>");
			return $this;
			}
		/**	Gets the Message, followed , if != "", by a horizontal line
			*	\brief gets all the messages
			*	\version 1.1.0
			*/
		function getMessages()
			{
			$ret=$this->Message;
			if($ret=="")return $ret;
				else return $ret."<hr/>";
			}
	/**	Initial session and arguments processing. Also sets the UserID & the Message.
		*	\brief Session and Arguments Processing.
		*	\version 1.1.1
		*	\return reference to $this
		*/
	function ProcessSessionAndArguments()
		{
		//error messages
		if(  isset( $_SESSION['message'] )  )
			{
			$this->AddMessage($_SESSION['message']);
			unset( $_SESSION['message']);
			}
		//default user
		if( ! isset( $_SESSION['userID'] )  )
			{
			$_SESSION['userID']=0;
			}
		$this->UserInfo=Users::UserInfo( $_SESSION['userID'] );
		//section
		if(  isset( $_REQUEST['section'] )  )
				$this->setSection( $_REQUEST['section'] );
			else
				$this->setSection( "Root" );
		// gets again the user because the (new) section may have changed the user(login/logout)
		$this->UserInfo=Users::UserInfo( $_SESSION['userID'] );
		return $this;
		}
	/// the current section name
	private $SectionName="Root";
	/**	\brief gets the name of the current section.
		*	\return the current section string
		*/
	function getSectionName()
		{
		return $this->SectionName;
		}
	/// the current PSection
	private $Section;
	/**	\brief gets the current section.
		*	\return the current section
		*/
	function getSection()
		{
		return $this->Section;
		}
	/**	Tries to change the current section to sectionName. if sectionName exists and the current user has access to it the current section is changed.
		*	\brief set the current section
		*	\version 1.1.0
		*	\param $sectionName the new section name
		*	\return reference to $this
		*/
	function & setSection($sectionName)
		{
		if(
			Sections::Exists($sectionName) &&
			Users::hasAccess($_SESSION['userID'],$sectionName) )
				$this->SectionName=$sectionName;
			else
				{
				$this->SectionName="Root";
				$this->AddMessage("section '$sectionName' is not accessible");
				}
		$sectionName=$this->getSectionName();
		$filename = "sections/$sectionName";
		if(file_exists($filename.".html"))
				$this->Section= new PStaticSection($sectionName);
			elseif(class_exists($sectionName,true))//tries to autoload the class
				{
				try
					{
					$this->Section= new $sectionName();
					}
				catch(Exception $e)
					{
					$this->AddMessage( $e->getMessage() );
					$this->Section= new PSection();
					}
				}
			else
				$this->Section= new PSection();
		return $this;
		}
	/// name of the Root section
	private $RootName="&#x27F0;";
		/**	Sets the name of the Root section because some programmers might want to call it "home page", "home", "first page" etc.
			*	\brief Sets the name of the Root section
			*	\param rn new root name
			*	\return reference to $this
			*/
		function & setRootName($rn="&#x27F0;")
			{
			$this->RootName=$rn;
			return $this;
			}
		/**	Gets the name of the Root section.
			*	\brief Get the Root Name.
			*	\return the Root Name
			*/
		function getRootName()
			{
			return $this->RootName;
			}
	/// the title of the webpage
	private $Title="PageTitle";
		/**	Sets the Title of the webpage to $title.
			*	\brief sets the title.
			*	\param $title the new title
			*	\return reference to $this
			*/
		function & setTitle($title)
			{
			$this->Title=$title;
			return $this;
			}
		/**	Gets the title of the webpage.
			*	\brief gets the title
			*	\return the title
			*/
		function getTitle()
			{
			return $this->Title;
			}
		/**	Gets the title of the webpage formated for the header.
			*	\brief gets the formated title
			*	\return the formated title
			*/
		function getFormatedTitle()
			{
			return new PText($this->getTitle());
			}
	/// the motto of the webpage
	private $Motto="PageMotto";
		/**	Sets the Motto of the webpage to $title.
			*	\brief sets the motto.
			*	\param $motto the new motto
			*	\return reference to $this
			*/
		function & setMotto($motto)
			{
			$this->Motto=$motto;
			return $this;
			}
		/**	Gets the Motto of the webpage.
			*	\brief gets the motto
			*	\return the Motto
			*/
		function getMotto()
			{
			return $this->Motto;
			}
	///language of the webpage, by default <b>en</b>
	private $Language="en";
		/**	Sets Language to lang.
			*	\brief set the language of the webpage
			*	\param $lang the language code
			*	\return reference to $this
			*/
		function & setLanguage($lang="en")
			{
			$this->Language=$lang;
			return $this;
			}
		/**	gets the Language.
			*	\brief gets the language of the webpage.
			*	\return the language code
			*/
		function getLanguage()
			{
			return $this->Language;
			}
		/**	The translate gadget from google is a script.
			*	\brief makes the <i>Translate Page</i> element.
			*	\return the translate gadget from google
			*/
		protected function TranslateElement()
			{
			return new PText(
'<div id="google_translate_element"></div><script>
function googleTranslateElementInit() {
new google.translate.TranslateElement({
	pageLanguage: \''.$this->getLanguage().'\'
}, \'google_translate_element\');
}
</script><script src="http://translate.google.com/translate_a/element.js?cb=googleTranslateElementInit"></script>');
			}
		/**	The translate gadget from google is a script.
			*	\brief <i>Translate Page</i> element with Public check.
			*	\return the TranslateElement() if the current section is member of public or Guest
			*/
		protected function TranslateElementWithCheck()
			{
			$row = Main()->DB->fetch_array_query(
				"
				SELECT COUNT(*)>0 '0'
				FROM Groups, GroupSections
				WHERE
					GroupID = Groups.ID  and
					SectionName ='".$this->getSectionName()."' and
					( Groups.Name = 'Guest' or Groups.Name = 'Public' )
				"
				);
			if($row[0]==1)// checks the '0' field returned by the query
					return $this->TranslateElement();
				else
					return null;
			}
	/// text "Bookmarks" in local language
	public $BookmarksTitle="Bookmarks";
	/// text "Website's sections" in local language
	public $WebsiteSectionsTitle="Website's sections";
	/// text "Subsections of" in local language
	public $SubsectionsTitle="Subsections of";
	/// the keywords of the webpage
	private $Keywords;
		/**	Sets the keywords of the webpage to $keywords.
			*	\brief sets the keywords.
			*	\param $keywords the new keywords
			*	\return reference to $this
			*/
		function & setKeywords($keywords)
			{
			$this->Keywords=$keywords;
			return $this;
			}
		/**	Gets the keywords of the webpage.
			*	\brief gets the keywords
			*	\return the keywords
			*/
		function getKeywords()
			{
			return $this->Keywords;
			}
	/**	YY
		*	\brief builds the head.
		*	\version 1.1.0
		*	\return the head element
		*/
	protected function Head()
		{
		$ret=new PElement("head");
		// the metas
			$ret->Insert($this->Head_Metas());
		// insert the title
			$ret->Insert(
				Object(new PElement("title"))
					->Insert(new PText( $this->getTitle()." : ".$this->getSection()->getTitle() ))
				);
		// the CSS theme
			$ret->Insert($this->Head_Css());
		// section's head
			$ret->Insert( $this->getSection()->Head() );
		return $ret;
		}
		/**	Gives <b>meta</b> PElement's inserted into a simple PContent object.
			*	\brief mets.
			*	\return a PContent object containing the <b>meta</b> html elements content, robots and keywords.
			*/
		protected function Head_Metas()
			{
			$Meta=new PStructure( new PSimpleTag("meta"), new PContent() );
				$Meta->newKid()
					->setAtr("http-equiv", "Content-Type")
					->setAtr("content", "application/xhtml+xml;charset=utf-8");
				$Meta->newKid()
					->setAtr("name","robots")
					->setAtr("content","all");
				$Meta->newKid()
					->setAtr("name","keywords")
					->setAtr("content",$this->getKeywords());
			return $Meta;
			}
		/**	Gives the <b>style</b> tag with the text/css type.
			*	\brief the css.
			*	\return \<style type="text/css"\> tag
			*/
		protected function Head_Css()
			{
			return
				Object(new PElement("style"))
						->setAtr("type","text/css")
						->setAtr("title","currentStyle")
						->setAtr("media","screen")
					;
			}
	/**	the body.
		*	\brief builds the body
		*	\version 1.0.0
		*	\return the <b>body</b> element
		*/
	protected function Body()
		{
		$ret=Object(new PElement("body"));
		return $ret;
		}
	/**	Gives a search engine form.
		*	\brief a search engine form.
		*	\return a PText containing the form.
		*/
	protected function SearchForm()
		{
		return new PText('<form action="http://www.google.com/cse" id="cse-search-box"><div><input type="hidden" name="cx" value="partner-pub-7129915672699792:a172f3-rgi4" /><input type="hidden" name="ie" value="ISO-8859-1" /><input type="text" name="q" /><input type="submit" name="sa" value="Search" /></div></form><script type="text/javascript" src="http://www.google.com/coop/cse/brand?form=cse-search-box&amp;lang=en"></script>');
		}
	/**	Gives a long height add from google.
		*	\brief a long height add.
		*	\return a PText containing the add
		*/
	protected function SkyAdd()
		{
		return new PText('<script type="text/javascript"><!--'."\n".
		'google_ad_client = "pub-7129915672699792";/* pwision 160x600, creat 01.05.2009 */ google_ad_slot = "6353909366";google_ad_width = 160;google_ad_height = 600;//--> </script><script type="text/javascript" src="http://pagead2.googlesyndication.com/pagead/show_ads.js"></script>');
		}
	/**	Gives a long weight add from google.
		*	\brief a long wheight add.
		*	\return a PText containing the add
		*/
	protected function LongAdd()
		{
		return new PText('<script type="text/javascript"><!--'."\n".
		'google_ad_client = "pub-7129915672699792";google_ad_slot = "0902253010";google_ad_width = 728;google_ad_height = 90;//--></script><script type="text/javascript" src="http://pagead2.googlesyndication.com/pagead/show_ads.js"></script>');
		}
	}

?>