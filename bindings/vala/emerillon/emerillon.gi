<?xml version="1.0"?>
<api version="1.0">
	<namespace name="Emerillon">
		<struct name="EmerillonSidebar">
			<method name="add_page" symbol="emerillon_sidebar_add_page">
				<return-type type="void"/>
				<parameters>
					<parameter name="sidebar" type="EmerillonSidebar*"/>
					<parameter name="title" type="gchar*"/>
					<parameter name="main_widget" type="GtkWidget*"/>
				</parameters>
			</method>
			<method name="get_n_pages" symbol="emerillon_sidebar_get_n_pages">
				<return-type type="gint"/>
				<parameters>
					<parameter name="sidebar" type="EmerillonSidebar*"/>
				</parameters>
			</method>
			<method name="is_empty" symbol="emerillon_sidebar_is_empty">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="sidebar" type="EmerillonSidebar*"/>
				</parameters>
			</method>
			<method name="new" symbol="emerillon_sidebar_new">
				<return-type type="GtkWidget*"/>
			</method>
			<method name="remove_page" symbol="emerillon_sidebar_remove_page">
				<return-type type="void"/>
				<parameters>
					<parameter name="sidebar" type="EmerillonSidebar*"/>
					<parameter name="main_widget" type="GtkWidget*"/>
				</parameters>
			</method>
			<method name="set_page" symbol="emerillon_sidebar_set_page">
				<return-type type="void"/>
				<parameters>
					<parameter name="sidebar" type="EmerillonSidebar*"/>
					<parameter name="main_widget" type="GtkWidget*"/>
				</parameters>
			</method>
			<field name="base_instance" type="GtkVBox"/>
			<field name="priv" type="EmerillonSidebarPrivate*"/>
		</struct>
		<struct name="EmerillonSidebarClass">
			<field name="base_class" type="GtkVBoxClass"/>
			<field name="page_added" type="GCallback"/>
			<field name="page_removed" type="GCallback"/>
		</struct>
		<struct name="EmerillonWindow">
			<method name="dup_default" symbol="emerillon_window_dup_default">
				<return-type type="GtkWidget*"/>
			</method>
			<method name="get_map_view" symbol="emerillon_window_get_map_view">
				<return-type type="ChamplainView*"/>
				<parameters>
					<parameter name="window" type="EmerillonWindow*"/>
				</parameters>
			</method>
			<method name="get_sidebar" symbol="emerillon_window_get_sidebar">
				<return-type type="GtkWidget*"/>
				<parameters>
					<parameter name="window" type="EmerillonWindow*"/>
				</parameters>
			</method>
			<method name="get_statusbar" symbol="emerillon_window_get_statusbar">
				<return-type type="GtkWidget*"/>
				<parameters>
					<parameter name="window" type="EmerillonWindow*"/>
				</parameters>
			</method>
			<method name="get_toolbar" symbol="emerillon_window_get_toolbar">
				<return-type type="GtkWidget*"/>
				<parameters>
					<parameter name="window" type="EmerillonWindow*"/>
				</parameters>
			</method>
			<method name="get_ui_manager" symbol="emerillon_window_get_ui_manager">
				<return-type type="GtkUIManager*"/>
				<parameters>
					<parameter name="window" type="EmerillonWindow*"/>
				</parameters>
			</method>
			<field name="parent" type="GtkWindow"/>
			<field name="priv" type="EmerillonWindowPrivate*"/>
		</struct>
		<struct name="EmerillonWindowClass">
			<field name="parent_class" type="GtkWindowClass"/>
		</struct>
	</namespace>
</api>

