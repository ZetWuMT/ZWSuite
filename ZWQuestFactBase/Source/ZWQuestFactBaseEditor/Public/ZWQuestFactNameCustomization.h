// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

/**
 * 
 */
class ZWQUESTFACTBASEEDITOR_API FZWQuestFactNameCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

private:
	void OnSearchTextChanged(const FText& InText);
	void OnSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	FText GetCurrentSelectionText() const;
	void OnMenuOpenChanged(bool bIsOpen);

	TSharedPtr<IPropertyHandle> PropertyHandlePtr;
	TArray<TSharedPtr<FString>> OptionsSource;
	TArray<TSharedPtr<FString>> FilteredOptions;
	TSharedPtr<SListView<TSharedPtr<FString>>> ListView;
	TSharedPtr<SEditableTextBox> SearchBar;
	TSharedPtr<SComboButton> ComboButton;
};
